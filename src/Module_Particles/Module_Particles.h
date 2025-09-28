#pragma once
#include "Cool/Log/MessageId.h"
#include "Cool/Nodes/NodeId.h"
#include "Cool/Nodes/NodesGraph.h"
#include "Cool/Particles/ParticleSystem.h"
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {
class Module_Particles : public Module {
public:
    Module_Particles() = default;
    explicit Module_Particles(Cool::NodeId const& id_of_node_storing_particles_count, std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on);

    void update() override;
    void on_time_changed() override;
    void imgui_generated_shader_code_tab() override;

    [[nodiscard]] auto needs_to_rerender() const -> bool override
    {
        return Module::needs_to_rerender() || _needs_to_update_particles;
    };

    void set_simulation_shader_code(tl::expected<std::string, std::string> const& shader_code, bool for_testing_nodes, int dimension);
    void on_time_reset() override;

private:
    void render(wgpu::RenderPassEncoder render_pass, DataToPassToShader const&) override;
    void update_particles(DataToPassToShader const&);
    // auto create_particle_system() const -> std::optional<Cool::ParticleSystem>; // TODO(WebGPU)
    void update_particles_count_ifn();
    auto desired_particles_count() const -> size_t;
    void log_simulation_shader_error(tl::expected<void, Cool::ErrorMessage> const&) const;
    void request_particles_to_reset();
    void request_particles_to_update() { _needs_to_update_particles = true; }

private:
    // mutable std::optional<Cool::ParticleSystem> _particle_system{}; // TODO(WebGPU)
    int                                         _particle_system_dimension{};
    // ModuleDependencies                          _depends_on{}; // TODO(Particles) Two dependencies, one for each shader (simulation and render)
    Cool::NodeId            _id_of_node_storing_particles_count{};
    bool                    _needs_to_update_particles{true};
    bool                    _force_init_particles{true};
    mutable Cool::MessageId _simulation_shader_error_id{};
    mutable std::string     _shader_code{};

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
