#include "Module_Particles.h"
#include <img/src/SizeU.h>
#include <imgui.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "Cool/Log/message_console.hpp"
#include "Cool/Midi/MidiChannel.h"
#include "Cool/Midi/MidiManager.h"
#include "Cool/OSC/OSCManager.h"
#include "Module/ShaderBased/set_uniforms_for_shader_based_module.hpp"
#include "Nodes/Node.h"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}
Module_Particles::Module_Particles()
    : _render_shader{Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
                         .kind        = Cool::ShaderKind::Vertex,
                         .source_code = fmt::format("#version 430\n{}", *Cool::File::to_string(Cool::Path::root() / "res/Particles/vertex.vert")),
                     }},
                     Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
                         .kind        = Cool::ShaderKind::Fragment,
                         .source_code = *Cool::File::to_string(Cool::Path::root() / "res/Particles/fragment.frag"),
                     }}}
{}

Module_Particles::Module_Particles(Cool::NodeId const& id_of_node_storing_particles_count, std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on)
    : Module{
          fmt::format("Particles {}", module_id()),
          std::move(texture_name_in_shader),
          std::move(modules_that_we_depend_on),
          std::move(nodes_that_we_depend_on)
      }
    , _id_of_node_storing_particles_count{id_of_node_storing_particles_count}

    , _render_shader{Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
                         .kind        = Cool::ShaderKind::Vertex,
                         .source_code = fmt::format("#version 430\n{}", *Cool::File::to_string(Cool::Path::root() / "res/Particles/vertex.vert")),
                     }},
                     Cool::OpenGL::ShaderModule{Cool::ShaderDescription{
                         .kind        = Cool::ShaderKind::Fragment,
                         .source_code = *Cool::File::to_string(Cool::Path::root() / "res/Particles/fragment.frag"),
                     }}}
{
    glpp::bind_vertex_array(_render_vao);
    glpp::bind_vertex_buffer(_render_vbo);
    glpp::set_vertex_buffer_attribute(_render_vbo, 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);                          // Vertices positions
    glpp::set_vertex_buffer_attribute(_render_vbo, 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // Vertices UVs
    glpp::set_vertex_buffer_data(
        _render_vbo, glpp::DataAccessFrequency::Static,
        std::array{
            -1.f, -1.f, 0.0f, 0.0f,
            +1.f, -1.f, 1.0f, 0.0f,
            +1.f, +1.f, 1.0f, 1.0f,

            -1.f, -1.f, 0.0f, 0.0f,
            +1.f, +1.f, 1.0f, 1.0f,
            -1.f, +1.f, 0.0f, 1.0f
        }
    );
}

void Module_Particles::spawn_particle(glm::vec2 pos) const
{
    _particles.push_back(Particle{
        .size     = 0.f,
        .size_max = _particle_size,
        .age      = 0.f,
        .duration = _particle_duration,
        .position = pos,
    });

    // _particle_to_init = _next_particle_to_init;
    // _next_particle_to_init++;
    // _next_particle_to_init = _next_particle_to_init % _particle_system->particles_count();

    // _particle_init_pos = pos;
}

void Module_Particles::imgui_windows(Ui_Ref) const
{
    ImGui::Begin("Particles");
    static glm::vec2 pos{0.f};
    if (ImGui::Button("Spawn Particle"))
    {
        spawn_particle(pos);
    }
    // ImGui::SliderFloat2("Pos", glm::value_ptr(pos), -1.f, 1.f);
    // ImGui::SliderFloat("Size", &_particle_size, _particle_size_min, _particle_size_max);
    // ImGui::SliderFloat("Duration", &_particle_duration, _particle_duration_min, _particle_duration_max);

    ImGui::SliderFloat("Fade pow", &_fade_pow, 1.f / 3.f, 3.f, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImGui::NewLine();

    ImGui::InputInt("Size MIDI Chan", &_midi_chan_size);
    ImGui::SliderFloat("Size Min", &_particle_size_min, 0.f, 1.f);
    ImGui::SliderFloat("Size Max", &_particle_size_max, 0.f, 2.f);

    ImGui::NewLine();

    ImGui::InputInt("Duration MIDI Chan", &_midi_chan_duration);
    ImGui::SliderFloat("Duration Min", &_particle_duration_min, 0.f, 5.f);
    ImGui::SliderFloat("Duration Max", &_particle_duration_max, 0.f, 10.f);

    _particle_size     = glm::mix(_particle_size_min, _particle_size_max, Cool::midi_manager().get_value(Cool::MidiChannel{.index = _midi_chan_size, .kind = Cool::MidiChannelKind::Slider}));
    _particle_duration = glm::mix(_particle_duration_min, _particle_duration_max, Cool::midi_manager().get_value(Cool::MidiChannel{.index = _midi_chan_duration, .kind = Cool::MidiChannelKind::Slider}));

    // for (auto const& particle : _particles)
    // {
    //     ImGui::TextUnformatted(fmt::format("{}", particle.age).c_str());
    // }
    ImGui::End();
}

void Module_Particles::on_time_reset()
{
    // if (_particle_system.has_value())
    //     request_particles_to_reset();
}

void Module_Particles::set_simulation_shader_code(tl::expected<std::string, std::string> const&, bool /* for_testing_nodes */, int)
{
    _depends_on      = {};
    _depends_on.time = true; // Particle modules always depend on time
}

auto Module_Particles::desired_particles_count() const -> size_t
{
    static constexpr size_t default_particles_count{1000};

    if (!_nodes_graph)
        return default_particles_count;

    auto const* const maybe_node = _nodes_graph->try_get_node<Node>(_id_of_node_storing_particles_count);
    if (!maybe_node)
        return default_particles_count;
    return maybe_node->particles_count().value_or(default_particles_count);
}
// auto Module_Particles::desired_particles_size() const -> float
// {
//     static constexpr float default_particles_count{0.3f};

//     if (!_nodes_graph)
//         return default_particles_count;

//     auto const* const maybe_node = _nodes_graph->try_get_node<Node>(_id_of_node_storing_particles_count);
//     if (!maybe_node)
//         return default_particles_count;
//     return maybe_node->particles_count().value_or(default_particles_count);
// }

void Module_Particles::log_simulation_shader_error(tl::expected<void, Cool::ErrorMessage> const& error) const
{
    log_module_error(error, _simulation_shader_error_id);
}

void Module_Particles::update_particles_count_ifn()
{
    // if (!_particle_system)
    //     return;
    // auto const particles_count = desired_particles_count();
    // if (particles_count == _particle_system->particles_count())
    //     return;
    // _particle_system->set_particles_count(particles_count); // TODO(History) Change through command
    // request_particles_to_reset();
}

void Module_Particles::request_particles_to_reset()
{
    // _particle_system->reset();
    // request_particles_to_update();
    // _force_init_particles = true;
}

void Module_Particles::update()
{
    update_particles_count_ifn();
    float const id = Cool::osc_manager().get_value({"/ID"});
    if (id != _last_osc_id)
    {
        _last_osc_id = id;
        spawn_particle({
            Cool::osc_manager().get_value({"/clickX"}),
            Cool::osc_manager().get_value({"/clickY"}),
        });
    }
}

void Module_Particles::on_time_changed()
{
    request_particles_to_update();
}

void Module_Particles::update_particles(DataToPassToShader const& data)
{
    std::erase_if(_particles, [](Particle const& particle) { return particle.remove_next_time; });
    for (auto& particle : _particles)
    {
        particle.age += data.system_values.delta_time.as_seconds_float();
        particle.size = (particle.age / particle.duration) * particle.size_max;
        if (particle.age > particle.duration)
        {
            particle.age              = particle.duration;
            particle.remove_next_time = true;
        }
    }
}

void Module_Particles::imgui_generated_shader_code_tab()
{
    // if (ImGui::BeginTabItem(fmt::format("{} (Simulation)", name()).c_str()))
    // {
    //     if (Cool::ImGuiExtras::input_text_multiline("##Particles simulation", &_shader_code, ImVec2{-1.f, -1.f}))
    //         set_simulation_shader_code(_shader_code, false, _particle_system ? _particle_system->dimension() : _particle_system_dimension);
    //     ImGui::EndTabItem();
    // }
}

void Module_Particles::render(DataToPassToShader const& data)
{
    // if (!_particle_system)
    //     return;

    if (_needs_to_update_particles)
        update_particles(data);

    // #if !defined(COOL_PARTICLES_DISABLED_REASON)
    render_target().set_size(data.system_values.render_target_size);
    render_target().render([&]() {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        // set_uniforms_for_shader_based_module(_particle_system->render_shader(), _depends_on, data, modules_that_we_depend_on(), nodes_that_we_depend_on());

        auto const view_proj_matrix_2D_mat3 = data.system_values.camera_2D.projection_matrix(1.f / img::aspect_ratio(data.system_values.render_target_size));

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        _render_shader.bind();
        _render_shader.set_uniform("view_proj_matrix", view_proj_matrix_2D_mat3);

        for (auto const& particle : _particles)
        {
            _render_shader.set_uniform("u_size", particle.size);
            _render_shader.set_uniform("u_fade_pow", _fade_pow);
            _render_shader.set_uniform("u_position", particle.position);
            _render_shader.set_uniform("u_fade", 1. - particle.age / particle.duration);

            _render_shader.bind();
            glpp::bind_vertex_array(_render_vao);
            glpp::draw_arrays(_render_vao, glpp::PrimitiveDrawMode::Triangles, 0, 6);
        }
    });
    // #endif
}

} // namespace Lab
