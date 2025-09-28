#pragma once
#include "Cool/Gpu/FullscreenPipeline.h"
#include "Cool/Log/MessageId.h"
#include "Cool/Nodes/NodesGraph.h"
#include "Module/Module.h"

namespace Lab {

class Module_Compositing : public Module {
public:
    Module_Compositing() = default;
    Module_Compositing(std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on);

    void update() override;
    void imgui_generated_shader_code_tab() override;

    void reset_shader();

    void set_shader_code(tl::expected<std::string, std::string> const& shader_code);

private:
    void render(wgpu::RenderPassEncoder render_pass, DataToPassToShader const&) override;
    void log_shader_error(tl::expected<void, Cool::ErrorMessage> const&) const;

private:
    mutable std::string              _shader_code{};
    mutable std::optional<Cool::FullscreenPipelineGLSL> _pipeline{};          // TODO(WebGPU)
    std::optional<Cool::BindGroup>                      _bind_group{};        // TODO(WebGPU) Move these 3 optionals into a single optional struct
    std::optional<Cool::BindGroupLayout>                _bind_group_layout{}; // TODO(WebGPU)
    mutable Cool::MessageId          _shader_error_id{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Base Module", ser20::base_class<Module>(this))
        );
    }
};

} // namespace Lab
