#include "Module_FeedbackLoop.hpp"
#include "Cool/Gpu/OpenGL/copy_tex_pipeline.hpp"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_FeedbackLoop::Module_FeedbackLoop(std::string texture_name_in_shader, std::shared_ptr<Module> module_that_we_depend_on)
    : Module{
          fmt::format("Feedback Loop {}", module_id()),
          std::move(texture_name_in_shader),
          {std::move(module_that_we_depend_on)},
          {} // We don't depend on any node
      }
{}

void Module_FeedbackLoop::on_time_reset()
{
    _renders_count = 0;
}

auto Module_FeedbackLoop::texture() const -> Cool::TextureRef
{
    auto const b = _renders_count < 2 ? !_render_target_ping_pong : _render_target_ping_pong; // During the first frame, the previous frame texture is not init, so use the current frame texture instead
    return (b ? _render_target : render_target()).texture_ref();
}

void Module_FeedbackLoop::render(DataToPassToShader const& data)
{
    _render_target_ping_pong = !_render_target_ping_pong;
    _renders_count++;
    auto& rt = _render_target_ping_pong ? render_target() : _render_target;
    rt.set_size(data.system_values.render_target_size);
    rt.render([&]() {
        // TODO(WebGPU) use a texture copy operation instead, it will be more efficient
        Cool::copy_tex_pipeline().shader()->bind();
        Cool::copy_tex_pipeline().shader()->set_uniform_texture("tex_to_copy", modules_that_we_depend_on()[0]->texture().id);
        glDisable(GL_BLEND);
        Cool::copy_tex_pipeline().draw();
        glEnable(GL_BLEND);
    });
}

void Module_FeedbackLoop::before_module_graph_renders()
{
    _rerender_this_frame = _rerender_next_frame;
    _rerender_next_frame = Module::needs_to_rerender(); // Make sure we will also render one frame after our dependencies rendered
}

auto Module_FeedbackLoop::needs_to_rerender() const -> bool
{
    return Module::needs_to_rerender() || _rerender_this_frame;
}

} // namespace Lab
