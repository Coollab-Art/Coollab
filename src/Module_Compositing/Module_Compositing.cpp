#include "Module_Compositing.h"
#include "Cool/Log/ErrorMessage.hpp"
#include "Cool/Log/message_console.hpp"
#include "Module/ShaderBased/set_uniforms_for_shader_based_module.hpp"
#include <webgpu/webgpu.hpp>
#include "Cool/WebGPU/FullscreenPipelineGLSL.h"
#include "Module/ShaderBased/make_system_bind_group.hpp"
#include "Module/ShaderBased/make_system_bind_group_layout.hpp"
#include "tl/expected.hpp"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_Compositing::Module_Compositing(std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on)
    : Module{
          fmt::format("Compositing {}", module_id()),
          std::move(texture_name_in_shader),
          std::move(modules_that_we_depend_on),
          std::move(nodes_that_we_depend_on)
      }
{
}

void Module_Compositing::update()
{
}

void Module_Compositing::reset_shader()
{
    _pipeline.reset(); // Make sure the shader will be empty if the compilation fails.
    _shader_code = "";
    _depends_on  = {};
    Cool::message_console().remove(_shader_error_id); // Make sure the error is removed if for some reason we don't compile the code (e.g. when there is no main node).
}

void Module_Compositing::set_shader_code(tl::expected<std::string, std::string> const& shader_code)
{
    _pipeline.reset();
    if (!shader_code)
    {
        log_shader_error(tl::make_unexpected(Cool::ErrorMessage{shader_code.error()})); // TODO(Logs) should be a notification
        return;
    }

    _shader_code = *shader_code;

    _depends_on = {};
    update_dependencies_from_shader_code(_depends_on, _shader_code); // Must be done before creating the bind group layout because it depends on it
    _bind_group_layout = make_system_bind_group_layout(_depends_on);

    auto maybe_pipeline = Cool::make_fullscreen_pipeline_glsl({.fragment_shader_module_creation_args = {
                                                                   .label = "Compositing Module fragment shader",
                                                                   .code  = _shader_code,
                                                               },
                                                               .extra_bind_group_layout = &*_bind_group_layout});
    if (maybe_pipeline.has_value())
        _pipeline = std::move(maybe_pipeline.value());
    else
        log_shader_error(maybe_pipeline.error());

    needs_to_rerender_flag().set_dirty();
}

void Module_Compositing::log_shader_error(tl::expected<void, Cool::ErrorMessage> const& maybe_err) const
{
    log_module_error(maybe_err, _shader_error_id);
}

void Module_Compositing::imgui_generated_shader_code_tab()
{
    if (ImGui::BeginTabItem(name().c_str()))
    {
        if (Cool::ImGuiExtras::input_text_multiline("##Compositing shader code", &_shader_code, ImVec2{-1.f, -1.f}))
            set_shader_code(_shader_code);
        ImGui::EndTabItem();
    }
}

void Module_Compositing::render(wgpu::RenderPassEncoder render_pass,DataToPassToShader const& data)
{
    if (!_pipeline.has_value())
        return;

    render_target().set_size(data.system_values.render_target_size);
   // render_target().render([&]() {
   //     glClearColor(0.f, 0.f, 0.f, 0.f);
   //     glClear(GL_COLOR_BUFFER_BIT);
  //      set_uniforms_for_shader_based_module(*_pipeline.shader(), _depends_on, data, modules_that_we_depend_on(), nodes_that_we_depend_on());
   //     _pipeline.draw();
  //  });
        set_uniforms_for_shader_based_module(*_pipeline, system_values, _depends_on, _feedback_double_buffer, *_nodes_graph);
    _pipeline->set_aspect_ratio_uniform(system_values.aspect_ratio());
    _bind_group = make_system_bind_group(*_bind_group_layout, system_values, _depends_on); // We need to keep the bind group alive until "the end of the frame" (until the render pass that we are filling in is executed)
    render_pass.setBindGroup(1, *_bind_group, 0, nullptr);
    _pipeline->draw(render_pass);
}

} // namespace Lab
