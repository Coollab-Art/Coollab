#include "Module_FluidSim.hpp"
#include <img/src/Size.h>
#include <imgui.h>
#include <cstdint>
#include <smart/smart.hpp>
#include "Cool/File/File.h"
#include "Cool/Gpu/FullscreenPipeline.h"
#include "Cool/Gpu/OpenGL/ComputeShader.h"
#include "Cool/Gpu/OpenGL/SSBO.h"
#include "Cool/TextureSource/TextureSamplerDescriptor.h"

namespace Lab {

void Module_FluidSim::init_sim()
{
    // _data->positions.bind();
    // _data->positions.upload_data(grid_size.pixels_count(), nullptr);
    _data->densities.bind();
    _data->densities.upload_data(_data->grid_size.pixels_count(), nullptr);

    _data->init_shader->bind();
    _data->init_shader->set_uniform("_grid_size", glm::uvec2{_data->grid_size.width(), _data->grid_size.height()});
    _data->init_shader->compute({_data->grid_size.pixels_count(), 1, 1});
}

void Module_FluidSim::update_sim(Cool::Time delta_time)
{
    _data->densities.bind();
    _data->update_shader->bind();
    _data->update_shader->set_uniform("_grid_size", glm::uvec2{_data->grid_size.width(), _data->grid_size.height()});
    _data->update_shader->set_uniform("_delta_time", delta_time.as_seconds_float());
    _data->update_shader->compute({_data->grid_size.pixels_count(), 1, 1});
}

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_FluidSim::Module_FluidSim(
    std::string               texture_name_in_shader,
    std::shared_ptr<Module>   module_that_we_depend_on,
    Cool::SharedVariable<int> glitch,
    Cool::SharedVariable<int> custom_resolution
)
    : Module{
          fmt::format("Fluid Simulation {}", module_id()),
          Cool::TextureFormat{.num_components = 4, .type = Cool::PixelType::UInt8},
          std::move(texture_name_in_shader),
          {std::move(module_that_we_depend_on)},
          {} // We don't depend on any node
      }
    , _glitch{std::move(glitch)}
    , _custom_resolution{std::move(custom_resolution)}
    , _data{std::make_unique<FluidSimData>()}
{
    _data->init_shader.emplace(64, *Cool::File::to_string(Cool::Path::root() / "res/FluidSim/init.comp"));
    _data->update_shader.emplace(64, *Cool::File::to_string(Cool::Path::root() / "res/FluidSim/update.comp"));
    _data->render_shader.compile(*Cool::File::to_string(Cool::Path::root() / "res/FluidSim/render.frag"));
}

void Module_FluidSim::on_time_reset()
{
    init_sim();
}

void Module_FluidSim::render(DataToPassToShader const& data)
{
    auto const grid_size = img::Size{static_cast<uint32_t>(100 * data.system_values.aspect_ratio()), 100};
    if (_data->grid_size != grid_size)
    {
        _data->grid_size = grid_size;
        init_sim();
    }
    update_sim(data.system_values.delta_time);
    render_target().set_size(data.system_values.render_target_size);
    render_target().render([&]() {
        _data->densities.bind();
        _data->render_shader.shader()->bind();
        _data->render_shader.shader()->set_uniform("_grid_size", glm::uvec2{grid_size.width(), grid_size.height()});
        // _data->render_shader.shader()->set_uniform("_aspect_ratio", data.system_values.aspect_ratio());
        // _data->render_shader.shader()->set_uniform("_camera2D_transform", data.system_values.camera_2D.transform_matrix());
        // _data->render_shader.set_uniform_texture("input_mask", modules_that_we_depend_on()[0]->texture().id);
        _data->render_shader.draw();
    });
}

} // namespace Lab
