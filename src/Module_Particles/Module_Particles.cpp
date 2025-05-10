#include "Module_Particles.h"
#include <glm/gtx/matrix_transform_2d.hpp>
#include "Cool/Log/message_console.hpp"
#include "Cool/OSC/OSCManager.h"
#include "Module/ShaderBased/set_uniforms_for_shader_based_module.hpp"
#include "Nodes/Node.h"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_Particles::Module_Particles(Cool::NodeId const& id_of_node_storing_particles_count, std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on)
    : Module{
          fmt::format("Particles {}", module_id()),
          std::move(texture_name_in_shader),
          std::move(modules_that_we_depend_on),
          std::move(nodes_that_we_depend_on)
      }
    , _id_of_node_storing_particles_count{id_of_node_storing_particles_count}
{
}

void Module_Particles::spawn_particle(glm::vec2 pos) const
{
    _particle_to_init = _next_particle_to_init;
    _next_particle_to_init++;
    _next_particle_to_init = _next_particle_to_init % _particle_system->particles_count();

    _particle_init_pos = pos;
}

void Module_Particles::imgui_windows(Ui_Ref) const
{
    // ImGui::Begin("Particles");
    // if (ImGui::Button("Spawn Particle"))
    // {
    //     spawn_particle({0., 1.});
    // }
    // ImGui::End();
}

void Module_Particles::on_time_reset()
{
    if (_particle_system.has_value())
        request_particles_to_reset();
}

void Module_Particles::set_simulation_shader_code(tl::expected<std::string, std::string> const& shader_code, bool /* for_testing_nodes */, int dimension)
{
    if (!shader_code)
    {
        log_simulation_shader_error(tl::make_unexpected(Cool::ErrorMessage{shader_code.error()})); // TODO(Logs) should be a notification
        return;
    }

    _shader_code               = *shader_code;
    _particle_system_dimension = dimension;
    Cool::message_console().remove(_simulation_shader_error_id);

    // TODO(Particles) Don't recreate the particle system every time, just change the shader but keep the current position and velocity of the particles
    try
    {
        if (_particle_system.has_value())
        {
            assert(dimension == _particle_system->dimension());
            _particle_system->set_simulation_shader(_shader_code);
        }
        else
        {
            assert(dimension == 2 || dimension == 3);
            _particle_system = Cool::ParticleSystem{
                dimension,
                Cool::ParticlesShadersCode{
                    .simulation = *shader_code,
                    .init       = fmt::format(
                        "{}{}",
                        dimension == 3 ? "#define COOLLAB_PARTICLES_3D\n" : "#define COOLLAB_PARTICLES_2D\n",
                        *Cool::File::to_string(Cool::Path::root() / "res/Particles/init.comp")
                    ),
                    .vertex = fmt::format(
                        "#version 430\n{}{}",
                        dimension == 3 ? "#define COOLLAB_PARTICLES_3D\n" : "",
                        *Cool::File::to_string(Cool::Path::root() / "res/Particles/vertex.vert")
                    ),
                    .fragment = *Cool::File::to_string(Cool::Path::root() / "res/Particles/fragment.frag"),
                },
                desired_particles_count()
            };
        }
    }
    catch (Cool::Exception const& e)
    {
        log_simulation_shader_error(tl::make_unexpected(Cool::ErrorMessage{e.error_message()}));
        return;
    }
    _depends_on      = {};
    _depends_on.time = true; // Particle modules always depend on time
    update_dependencies_from_shader_code(_depends_on, _shader_code);
    request_particles_to_reset();
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

void Module_Particles::log_simulation_shader_error(tl::expected<void, Cool::ErrorMessage> const& error) const
{
    log_module_error(error, _simulation_shader_error_id);
}

void Module_Particles::update_particles_count_ifn()
{
    if (!_particle_system)
        return;
    auto const particles_count = desired_particles_count();
    if (particles_count == _particle_system->particles_count())
        return;
    _particle_system->set_particles_count(particles_count); // TODO(History) Change through command
    request_particles_to_reset();
}

void Module_Particles::request_particles_to_reset()
{
    _particle_system->reset();
    request_particles_to_update();
    _force_init_particles = true;
}

void Module_Particles::update()
{
    update_particles_count_ifn();
}

void Module_Particles::on_time_changed()
{
    request_particles_to_update();
}

void Module_Particles::update_particles(DataToPassToShader const& data)
{
    if (!_particle_system)
        return;

#if !defined(COOL_PARTICLES_DISABLED_REASON)
    if (DebugOptions::log_when_updating_particles())
        Cool::Log::info(name(), "Updated particles");

    float const id = Cool::osc_manager().get_value({"/ID"});
    if (id != _last_osc_id)
    {
        _last_osc_id = id;
        spawn_particle({
            Cool::osc_manager().get_value({"/clickX"}),
            Cool::osc_manager().get_value({"/clickY"}),
        });
    }

    _particle_system->simulation_shader().bind();
    _particle_system->simulation_shader().bind();
    _particle_system->simulation_shader().set_uniform("_particle_to_init", _particle_to_init.value_or(-1));
    _particle_system->simulation_shader().set_uniform("_particle_to_init_pos", _particle_init_pos.value_or(glm::vec2{}));
    _particle_to_init.reset();
    _particle_system->simulation_shader().set_uniform("_force_init_particles", _force_init_particles);
    set_uniforms_for_shader_based_module(_particle_system->simulation_shader(), _depends_on, data, modules_that_we_depend_on(), nodes_that_we_depend_on());
    _particle_system->update();
    _force_init_particles      = false;
    _needs_to_update_particles = false;
#else
    std::ignore = data;
#endif
}

void Module_Particles::imgui_generated_shader_code_tab()
{
    if (ImGui::BeginTabItem(fmt::format("{} (Simulation)", name()).c_str()))
    {
        if (Cool::ImGuiExtras::input_text_multiline("##Particles simulation", &_shader_code, ImVec2{-1.f, -1.f}))
            set_simulation_shader_code(_shader_code, false, _particle_system ? _particle_system->dimension() : _particle_system_dimension);
        ImGui::EndTabItem();
    }
}

void Module_Particles::render(DataToPassToShader const& data)
{
    if (!_particle_system)
        return;

    if (_needs_to_update_particles)
        update_particles(data);

#if !defined(COOL_PARTICLES_DISABLED_REASON)
    render_target().set_size(data.system_values.render_target_size);
    render_target().render([&]() {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        set_uniforms_for_shader_based_module(_particle_system->render_shader(), _depends_on, data, modules_that_we_depend_on(), nodes_that_we_depend_on());

        auto const view_proj_matrix_2D_mat3 = data.system_values.camera_2D_view_projection_matrix();
        auto const view_proj_matrix_2D_mat4 = glm::mat4{
            glm::vec4{view_proj_matrix_2D_mat3[0], 0.f},
            glm::vec4{view_proj_matrix_2D_mat3[1], 0.f},
            glm::vec4{0.f},
            glm::vec4{view_proj_matrix_2D_mat3[2][0], view_proj_matrix_2D_mat3[2][1], 0.f, view_proj_matrix_2D_mat3[2][2]}
        };

        if (_particle_system->dimension() == 2)
        {
            _particle_system->render_shader().set_uniform("view_proj_matrix", view_proj_matrix_2D_mat4);
        }
        else if (_particle_system->dimension() == 3)
        {
            _particle_system->render_shader().set_uniform("view_proj_matrix", view_proj_matrix_2D_mat4 * data.system_values.camera_3D.view_projection_matrix(1.f /* The aspect ratio is already taken into account in the camera 2D matrix */));
            _particle_system->render_shader().set_uniform("cool_camera_view", data.system_values.camera_3D.view_matrix());
        }
        _particle_system->render();
    });
#endif
}

} // namespace Lab
