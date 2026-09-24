#include "Module_JFA.hpp"
#include <imgui.h>
#include <cstdint>
#include <smart/smart.hpp>
#include "Cool/File/File.h"
#include "Cool/Gpu/FullscreenPipeline.h"
#include "Cool/TextureSource/TextureSamplerDescriptor.h"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

static constexpr auto texture_format = Cool::TextureFormat{.num_components = 4, .type = Cool::PixelType::Float16};

/// A shader that fails to load or compile here leaves the pipeline empty, and FullscreenPipeline::draw()
/// then silently does nothing. So we must report the failure, otherwise the only symptom is a black image.
static auto make_shader(std::filesystem::path const& path) -> Cool::FullscreenPipeline
{
    auto shader = Cool::FullscreenPipeline{};
    auto source = Cool::File::to_string(Cool::Path::root() / path);
    if (!source)
    {
        Cool::Log::error("JFA", fmt::format("Failed to read \"{}\":\n{}", path, source.error()));
        return shader;
    }
    auto const compilation = shader.compile(*source);
    if (!compilation)
        Cool::Log::error("JFA", fmt::format("Failed to compile \"{}\":\n{}", path, compilation.error().message));
    return shader;
}

static auto init_shader() -> Cool::FullscreenPipeline&
{
    static auto instance = make_shader("res/JFA/init.frag");
    return instance;
}

static auto flood_step_shader() -> Cool::FullscreenPipeline&
{
    static auto instance = make_shader("res/JFA/flood_step.frag");
    return instance;
}

static auto convert_to_image_shader() -> Cool::FullscreenPipeline&
{
    static auto instance = make_shader("res/JFA/convert_to_image.frag");
    return instance;
}

Module_JFA::Module_JFA(
    std::string               texture_name_in_shader,
    std::shared_ptr<Module>   module_that_we_depend_on,
    Cool::SharedVariable<int> glitch,
    Cool::SharedVariable<int> custom_resolution
)
    : Module{
          fmt::format("Mask to Shape {}", module_id()),
          texture_format,
          std::move(texture_name_in_shader),
          {std::move(module_that_we_depend_on)},
          {} // We don't depend on any node
      }
    , _render_target{texture_format}
    , _glitch{std::move(glitch)}
    , _custom_resolution{std::move(custom_resolution)}

{}

static auto find_smallest_power_of_2_greater_or_equal_to(uint32_t n) -> uint32_t
{
    if (n == 0)
        return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

auto Module_JFA::desired_size(img::Size render_target_size) const -> img::Size
{
    if (_is_main_module)
        return render_target_size;
    return desired_intermediate_size(render_target_size);
}

auto Module_JFA::desired_intermediate_size(img::Size render_target_size) const -> img::Size
{
    if (_custom_resolution.value() < 0)
    {
        // JFA needs a square texture with a size that is a power of 2
        auto const size = find_smallest_power_of_2_greater_or_equal_to(std::max(render_target_size.width(), render_target_size.height()));
        return {size, size};
    }
    else // NOLINT(*else-after-return)
    {
        auto const size = smart::keep_above(1u, static_cast<uint32_t>(_custom_resolution.value()));
        return {size, size};
    }
}

static auto pow2(int n) -> int
{
    if (n < 0)
        return 1;
    return 1 << n;
}

void Module_JFA::render(DataToPassToShader const& data)
{
    auto const size = desired_intermediate_size(data.system_values.render_target_size);
    render_target().set_size(size);
    _render_target.set_size(size);
    render_target().render([&]() {
        init_shader().shader()->bind();
        init_shader().shader()->set_uniform("_aspect_ratio", data.system_values.aspect_ratio());
        init_shader().shader()->set_uniform("_camera2D_transform", data.system_values.camera_2D.transform_matrix());
        init_shader().shader()->set_uniform_texture("input_mask", modules_that_we_depend_on()[0]->texture().id);
        init_shader().draw();
        _read_on_default_rt = true;
    });
    uint32_t const resolution = std::max(size.width(), size.height()); // In case width != height (although this should not happen because it messes up JFA)
    int            jump_size  = static_cast<int>(resolution) / 2;
    while (jump_size >= pow2(_glitch.value())) // We are supposed to go until jump_size == 0, but we might stop N steps earlier to create some glitch (N = _glitch.value())
    {
        auto& read_rt  = _read_on_default_rt ? render_target() : _render_target;
        auto& write_rt = _read_on_default_rt ? _render_target : render_target();

        write_rt.render([&]() {
            flood_step_shader().shader()->bind();
            flood_step_shader().shader()->set_uniform_texture("prev_step", read_rt.texture_ref().id, Cool::TextureSamplerDescriptor{.repeat_mode = Cool::TextureRepeatMode::Clamp, .interpolation_mode = glpp::Interpolation::NearestNeighbour});
            flood_step_shader().shader()->set_uniform("resolution", resolution);
            flood_step_shader().shader()->set_uniform("jump_size", jump_size);
            flood_step_shader().shader()->set_uniform("_aspect_ratio", data.system_values.aspect_ratio());
            flood_step_shader().shader()->set_uniform("_camera2D_transform", data.system_values.camera_2D.transform_matrix());
            flood_step_shader().draw();
        });

        jump_size /= 2;
        _read_on_default_rt = !_read_on_default_rt;
    }

    if (_is_main_module)
    {
        auto& read_rt  = _read_on_default_rt ? render_target() : _render_target;
        auto& write_rt = _read_on_default_rt ? _render_target : render_target();
        write_rt.set_size(data.system_values.render_target_size);

        write_rt.render([&]() {
            convert_to_image_shader().shader()->bind();
            convert_to_image_shader().shader()->set_uniform_texture("closest_uv_tex", read_rt.texture_ref().id, Cool::TextureSamplerDescriptor{.repeat_mode = Cool::TextureRepeatMode::Clamp, .interpolation_mode = glpp::Interpolation::NearestNeighbour});
            convert_to_image_shader().shader()->set_uniform("_aspect_ratio", data.system_values.aspect_ratio());
            convert_to_image_shader().shader()->set_uniform("_camera2D_transform", data.system_values.camera_2D.transform_matrix());
            convert_to_image_shader().draw();
        });

        _read_on_default_rt = !_read_on_default_rt;
    }
}

auto Module_JFA::texture() const -> Cool::TextureRef
{
    return (_read_on_default_rt ? render_target() : _render_target).texture_ref();
}

} // namespace Lab
