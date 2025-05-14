#pragma once
#include "Cool/Log/MessageId.h"
#include "Cool/Nodes/NodeId.h"
#include "Cool/Nodes/NodesGraph.h"
#include "Cool/Particles/ParticleSystem.h"
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {

struct Particle {
    float     size;
    float     size_max;
    float     age;
    float     duration;
    glm::vec2 position;
    bool      remove_next_time{false};
};

class Module_Particles : public Module {
public:
    Module_Particles();
    explicit Module_Particles(Cool::NodeId const& id_of_node_storing_particles_count, std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on);

    void update() override;
    void on_time_changed() override;
    void imgui_generated_shader_code_tab() override;

    void imgui_windows(Ui_Ref) const override;

    [[nodiscard]] auto needs_to_rerender() const -> bool override
    {
        return Module::needs_to_rerender() || _needs_to_update_particles;
    };

    void set_simulation_shader_code(tl::expected<std::string, std::string> const& shader_code, bool for_testing_nodes, int dimension);
    void on_time_reset() override;

private:
    void render(DataToPassToShader const&) override;
    void update_particles(DataToPassToShader const&);
    auto create_particle_system() const -> std::optional<Cool::ParticleSystem>;
    void update_particles_count_ifn();
    auto desired_particles_count() const -> size_t;
    // auto desired_particles_size() const -> float;
    // auto desired_particles_duration() const -> float;
    void log_simulation_shader_error(tl::expected<void, Cool::ErrorMessage> const&) const;
    void request_particles_to_reset();
    void request_particles_to_update() { _needs_to_update_particles = true; }

    void spawn_particle(glm::vec2 pos) const;

private:
    // mutable std::optional<Cool::ParticleSystem> _particle_system{};
    int _particle_system_dimension{};
    // ModuleDependencies                          _depends_on{}; // TODO(Particles) Two dependencies, one for each shader (simulation and render)
    Cool::NodeId            _id_of_node_storing_particles_count{};
    bool                    _needs_to_update_particles{true};
    bool                    _force_init_particles{true};
    mutable Cool::MessageId _simulation_shader_error_id{};
    // mutable std::string              _shader_code{};
    mutable std::optional<int>       _particle_to_init{};
    mutable int                      _next_particle_to_init = 0;
    mutable std::optional<glm::vec2> _particle_init_pos{};
    float                            _last_osc_id = 0.f;
    mutable std::vector<Particle>    _particles;
    Cool::OpenGL::Shader             _render_shader;
    glpp::UniqueVertexArray          _render_vao{};
    glpp::UniqueBuffer               _render_vbo{};

    mutable float _particle_size{0.15f};
    mutable float _particle_size_min{0.f};
    mutable float _particle_size_max{1.5f};
    mutable float _particle_duration{1.f};
    mutable float _particle_duration_min{0.f};
    mutable float _particle_duration_max{5.f};
    mutable float _fade_pow{3.f};

    mutable int _midi_chan_size{19};
    mutable int _midi_chan_duration{23};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Base Module", ser20::base_class<Module>(this)),
            ser20::make_nvp("Size chan", _midi_chan_size),
            ser20::make_nvp("Size min", _particle_size_min),
            ser20::make_nvp("Size max", _particle_size_max),
            ser20::make_nvp("Duration chan", _midi_chan_duration),
            ser20::make_nvp("Duration min", _particle_duration_min),
            ser20::make_nvp("Duration max", _particle_duration_max),
            ser20::make_nvp("Fade pow", _fade_pow)
        );
    }
};

} // namespace Lab
