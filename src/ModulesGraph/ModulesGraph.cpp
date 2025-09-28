#include "ModulesGraph.h"
#include <algorithm>
#include "CommandCore/CommandExecutor.h"
#include "Cool/Audio/AudioManager.h"
#include "Cool/Nodes/NodesLibrary.h"
#include "Cool/StrongTypes/Camera2D.h"
#include "Cool/TextureSource/default_textures.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Module_Compositing/Module_Compositing.h"
#include "Module_Compositing/generate_compositing_shader_code.h"
#include "Module_Default/Module_Default.hpp"
#include "Module_FeedbackLoop/Module_FeedbackLoop.hpp"
#include "Module_Particles/Module_Particles.h"
#include "Module_Particles/generate_simulation_shader_code.h"
#include "Nodes/FunctionSignature.h"
#include "Nodes/valid_glsl.h"
#include "UI/imgui_show.h"
#include "imgui.h"

namespace Lab {

void ModulesGraph::update()
{
    for (auto const& module : _modules)
    {
        module->_nodes_graph = &_nodes_editor.graph();
        module->update();
    }
}

void ModulesGraph::check_for_rerender_and_rebuild(DataToPassToShader const& data_to_pass_to_shader, DataToGenerateShaderCode const& data_to_generate_shader_code)
{
    if (rebuild_modules_graph_flag().is_dirty())
    {
        if (DebugOptions::log_when_compiling_nodes())
            Cool::Log::info("Modules Graph", "Rebuilt");
        recreate_all_modules(_main_node_id, data_to_generate_shader_code);
        rebuild_modules_graph_flag().set_clean();
    }

    if (rerender_all_flag().is_dirty())
    {
        request_rerender_all();
        rerender_all_flag().set_clean();
    }

    for (auto const& module : _modules)
    {
        // Modules that depend on time_since_last_midi_button_pressed should rerender every frame
        if (module->depends_on().time_since_last_midi_button_pressed)
            module->needs_to_rerender_flag().set_dirty();

        // Rerender when render size changes
        if (data_to_pass_to_shader.system_values.render_target_size != module->texture().size)
            module->needs_to_rerender_flag().set_dirty();

        if (module->depends_on().always_rerender)
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::render(DataToPassToShader const& data_to_pass_to_shader, DataToGenerateShaderCode const& data_to_generate_shader_code)
{
    check_for_rerender_and_rebuild(data_to_pass_to_shader, data_to_generate_shader_code);

    for (auto& module : _modules)
    {
        module->_nodes_graph = &_nodes_editor.graph(); // TODO(Particles) Remove those _nodes_graph
        module->before_module_graph_renders();
    }

    render_module_ifn(*_root_module, data_to_pass_to_shader);
}

void ModulesGraph::render_module_ifn(Module& module, DataToPassToShader const& data)
{
    if (!module.needs_to_rerender())
        return;

    // Render all the dependencies first, so that we can use their textures
    for (auto const& prev : module.modules_that_we_depend_on())
        render_module_ifn(*prev, data);

    module.do_rendering(data);
    module.needs_to_rerender_flag().set_clean();

    if (DebugOptions::log_when_rendering())
        Cool::Log::info(module.name(), fmt::format("Rendered ({}x{})", data.system_values.render_target_size.width(), data.system_values.render_target_size.height()));
}

void ModulesGraph::request_rerender_all()
{
    for (auto& module : _modules)
        module->needs_to_rerender_flag().set_dirty();
}

void ModulesGraph::on_time_reset()
{
    for (auto& module : _modules)
        module->on_time_reset();
}

static auto texture_name_for_module(Cool::NodeId const& id) -> std::string
{
    return valid_glsl(fmt::format("texture_{})", to_string(id.underlying_uuid())));
}

enum class NodeModuleness {
    Generic,
    Particle,
    FeedbackLoop,
};

static auto is_feedback_loop(NodeDefinition const& node_definition)
{
    return node_definition.name() == "Feedback (One frame delay)";
}

static auto node_moduleness(NodeDefinition const& node_definition)
{
    if (is_particle(node_definition.signature()))
        return NodeModuleness::Particle;
    if (is_feedback_loop(node_definition))
        return NodeModuleness::FeedbackLoop;
    return NodeModuleness::Generic;
}

void ModulesGraph::clear_error_messages()
{
    for (auto const& node : _nodes_editor.graph().nodes())
    {
        for (auto const& any_input : node.second.downcast<Node>().value_inputs())
        {
            std::visit([&](auto&& input) {
                ImGuiNotify::close_immediately(input.notification_id());
            },
                       any_input);
        }
    }
}

void ModulesGraph::recreate_all_modules(Cool::NodeId const& root_node_id, DataToGenerateShaderCode const& data_to_generate_shader_code)
{
    _modules.clear();
    clear_error_messages();
    _root_module = create_module(root_node_id, data_to_generate_shader_code);
    request_rerender_all();
}

auto ModulesGraph::create_module(Cool::NodeId const& root_node_id, DataToGenerateShaderCode const& data) -> std::shared_ptr<Module>
{
    auto const* node = data.nodes_graph.try_get_node<Node>(root_node_id);
    if (!node)
        return create_default_module(); // TODO(Module) Return an error message? Probably not because this is legit, eg when a feedback loop has nothing in its input pin
    auto const* node_def = data.get_node_definition(node->id_names());
    if (!node_def)
        return create_default_module(); // TODO(Module) Return an error message, this shouldn't happen

    switch (node_moduleness(*node_def))
    {
    case NodeModuleness::Generic:
        return create_compositing_module(root_node_id, data);
    case NodeModuleness::Particle:
        return create_particles_module(root_node_id, *node_def, data);
    case NodeModuleness::FeedbackLoop:
        return create_feedback_loop_module(root_node_id, data);
    }
    assert(false);
    return create_default_module();
}

auto ModulesGraph::create_module_impl(std::string const& texture_name_in_shader, std::function<std::shared_ptr<Module>()> const& make_module) -> std::shared_ptr<Module>
{
    { // If the module already exists, just return it
        auto const it = std::find_if(_modules.begin(), _modules.end(), [&](auto&& module) {
            return module->texture_name_in_shader() == texture_name_in_shader;
        });
        if (it != _modules.end())
            return *it;
    }

    // Otherwise create it and add it to the list
    _modules.push_back(make_module());
    return _modules.back();
}

auto ModulesGraph::create_compositing_module(Cool::NodeId const& root_node_id, DataToGenerateShaderCode const& data) -> std::shared_ptr<Module>
{
    auto const texture_name_in_shader = texture_name_for_module(root_node_id);
    return create_module_impl(texture_name_in_shader, [&]() -> std::shared_ptr<Module> {
        auto modules_that_we_depend_on = std::vector<std::shared_ptr<Module>>{};
        auto nodes_that_we_depend_on   = std::vector<Cool::NodeId>{};

        auto const shader_code = generate_compositing_shader_code(
            root_node_id,
            [&](Cool::NodeId const& node_id, NodeDefinition const& node_definition) -> std::optional<std::string> {
                switch (node_moduleness(node_definition))
                {
                case NodeModuleness::Generic:
                {
                    nodes_that_we_depend_on.push_back(node_id);
                    return std::nullopt;
                }
                case NodeModuleness::Particle:
                {
                    modules_that_we_depend_on.push_back(create_particles_module(node_id, node_definition, data));
                    return modules_that_we_depend_on.back()->texture_name_in_shader();
                }
                case NodeModuleness::FeedbackLoop:
                {
                    modules_that_we_depend_on.push_back(create_feedback_loop_module(node_id, data));
                    return modules_that_we_depend_on.back()->texture_name_in_shader();
                }
                }
                assert(false);
                return std::nullopt;
            },
            [&]() {
                std::vector<std::string> tex_names;
                tex_names.reserve(_modules.size());
                for (auto const& module : _modules)
                {
                    tex_names.push_back(module->texture_name_in_shader());
                }
                return tex_names;
            },
            data
        );

        auto module = std::make_shared<Module_Compositing>(
            texture_name_in_shader, // Don't move it because it might still be used by create_module_impl()
            std::move(modules_that_we_depend_on),
            std::move(nodes_that_we_depend_on)
        );
        module->set_shader_code(shader_code);
        return module;
    });
}

auto ModulesGraph::create_particles_module(Cool::NodeId const& root_node_id, NodeDefinition const& node_definition, DataToGenerateShaderCode const& data) -> std::shared_ptr<Module>
{
    auto const texture_name_in_shader = texture_name_for_module(root_node_id);
    return create_module_impl(texture_name_in_shader, [&]() -> std::shared_ptr<Module> {
        auto modules_that_we_depend_on = std::vector<std::shared_ptr<Module>>{};
        auto nodes_that_we_depend_on   = std::vector<Cool::NodeId>{};

        int const dimension = is_particle_3D(node_definition.signature()) ? 3 : 2;

        auto       id_of_node_storing_particles_count = Cool::NodeId{}; // Will be initialized by generate_simulation_shader_code()
        auto const simulation_shader_code             = generate_simulation_shader_code(
            root_node_id,
            id_of_node_storing_particles_count,
            dimension,
            data,
            [&](Cool::NodeId const& node_id, NodeDefinition const& node_definition) -> std::optional<std::string> {
                switch (node_moduleness(node_definition))
                {
                case NodeModuleness::Generic:
                case NodeModuleness::Particle: // TODO(Particles) This is not quite right, a particle system might depend on the image generated by another particles module
                {
                    nodes_that_we_depend_on.push_back(node_id);
                    return std::nullopt;
                }
                case NodeModuleness::FeedbackLoop:
                {
                    modules_that_we_depend_on.push_back(create_feedback_loop_module(node_id, data));
                    return modules_that_we_depend_on.back()->texture_name_in_shader();
                }
                }
                assert(false);
                return std::nullopt;
            }
        );

        auto module = std::make_shared<Module_Particles>(
            id_of_node_storing_particles_count,
            texture_name_in_shader, // Don't move it because it might still be used by create_module_impl()
            std::move(modules_that_we_depend_on),
            std::move(nodes_that_we_depend_on)
        );
        module->set_simulation_shader_code(simulation_shader_code, false, dimension);
        return module;
    });
}

auto ModulesGraph::create_feedback_loop_module(Cool::NodeId const& root_node_id, DataToGenerateShaderCode const& data) -> std::shared_ptr<Module>
{
    auto const texture_name_in_shader = texture_name_for_module(root_node_id);
    return create_module_impl(texture_name_in_shader, [&]() -> std::shared_ptr<Module> {
        auto const* node = data.nodes_graph.try_get_node<Node>(root_node_id);
        if (!node)
            return create_default_module(); // TODO(Module) Return an error message? This should never happen

        assert(node->input_pins().size() == 1);
        auto const predecessor_node_id = data.nodes_graph.find_node_connected_to_input_pin(node->input_pins()[0].id());
        auto       dependency          = create_module(predecessor_node_id, data);

        return std::make_shared<Module_FeedbackLoop>(
            texture_name_in_shader, // Don't move it because it might still be used by create_module_impl()
            std::move(dependency)
        );
    });
}

auto ModulesGraph::create_default_module() -> std::shared_ptr<Module>
{
    auto const texture_name_in_shader = "texture_of_the_default_module"s;
    return create_module_impl(texture_name_in_shader, [&]() -> std::shared_ptr<Module> {
        return std::make_shared<Module_Default>(texture_name_in_shader);
    });
}

void ModulesGraph::imgui_windows(Ui_Ref ui, Cool::AudioManager& audio_manager, Cool::NodesLibrary const& nodes_library) const
{
    for (auto const& module : _modules)
        module->imgui_windows(ui);

    {
        auto cfg = Cool::NodesConfig{nodes_config(ui, audio_manager, nodes_library)};
        if (_nodes_editor.imgui_windows(cfg, nodes_library))
            rebuild_modules_graph_flag().set_dirty();
    }
    DebugOptions::show_generated_shader_code([&] {
        if (ImGui::BeginTabBar("Shaders Tabs", ImGuiTabBarFlags_None))
        {
            for (auto const& module : _modules)
            {
                ImGui::PushID(module.get());
                module->imgui_generated_shader_code_tab();
                ImGui::PopID();
            }
            ImGui::EndTabBar();
        }
    });
}

static auto make_gizmo(Cool::SharedVariable<Cool::Point2D> const& var, CommandExecutor const& command_executor, Cool::Camera2D const& cam_2D) -> Cool::Gizmo_Point2D
{
    return Cool::Gizmo_Point2D{
        .get_position =
            [=]() { return Cool::ViewCoordinates{glm::vec2{cam_2D.view_matrix() * glm::vec3{var.value().value, 1.f}}}; },
        .set_position =
            [=](Cool::ViewCoordinates pos) {
                auto const world_pos = glm::vec2{cam_2D.transform_matrix() * glm::vec3{pos, 1.f}};
                command_executor.execute(Command_SetVariable<Cool::Point2D>{.var_ref = var.get_ref(), .value = Cool::Point2D{world_pos}});
            },
        .on_drag_stop =
            [=]() { command_executor.execute(Command_FinishedEditingVariable{}); },
        .id = var.id(),
    };
}

void ModulesGraph::submit_gizmos(Cool::GizmoManager& gizmos, CommandExecutor const& command_executor, Cool::Camera2D const& cam_2D)
{
    _nodes_editor.for_each_selected_node([&](Cool::Node const& node, Cool::NodeId const&) {
        for (auto const& var : node.downcast<Node>().value_inputs())
        {
            if (auto const* point_2D_var = std::get_if<Cool::SharedVariable<Cool::Point2D>>(&var))
            {
                gizmos.push(make_gizmo(*point_2D_var, command_executor, cam_2D));
            }
        }
    });
}

auto ModulesGraph::final_texture() const -> Cool::TextureRef
{
    if (!_root_module)
        return Cool::transparent_texture().ref();
    return _root_module->texture();
}

auto ModulesGraph::nodes_config(Ui_Ref ui, Cool::AudioManager& audio_manager, Cool::NodesLibrary const& nodes_library) const -> NodesConfig
{
    return NodesConfig{
        Cool::GetNodeDefinition_Ref<NodeDefinition>{nodes_library},
        Cool::GetNodeCategoryConfig_Ref{nodes_library},
        ui,
        _main_node_id,
        _dirty_flags,
        _nodes_editor,
        audio_manager,
        ui.command_executor(),
    };
}

void ModulesGraph::on_time_changed()
{
    for (auto& module : _modules)
    {
        module->on_time_changed();
        if (module->depends_on().time)
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::on_audio_changed()
{
    for (auto& module : _modules)
    {
        if (module->depends_on().audio())
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::on_osc_channel_changed(Cool::OSCChannel const& osc_channel)
{
    for (auto& module : _modules)
    {
        if (module->depends_on().osc_channel(osc_channel))
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::on_midi_channel_changed(Cool::MidiChannel const& midi_channel)
{
    for (auto& module : _modules)
    {
        if (module->depends_on().midi_channel(midi_channel))
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::on_last_midi_button_pressed_changed()
{
    for (auto& module : _modules)
    {
        if (module->depends_on().last_midi_button_pressed)
            module->needs_to_rerender_flag().set_dirty();
    }
}

void ModulesGraph::update_dependencies_from_nodes_graph()
{
    for (auto const& module : _modules)
        module->update_dependencies_from_nodes_graph(_nodes_editor.graph());
}

void ModulesGraph::debug_show_nodes_and_links_registries_windows(Ui_Ref ui) const
{
    ui.window({.name = "Nodes Registry"}, [&]() {
        imgui_show(_nodes_editor.graph().nodes());
    });
    ui.window({.name = "Links Registry"}, [&]() {
        imgui_show(_nodes_editor.graph().links());
    });
}

auto ModulesGraph::get_main_node_id() const -> Cool::NodeId const&
{
    return _main_node_id;
}
void ModulesGraph::set_main_node_id(Cool::NodeId const& id)
{
    if (_main_node_id == id)
        return;
    _main_node_id = id;
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

void ModulesGraph::add_node(Cool::NodeId const& id, Node const& node)
{
    _nodes_editor.graph().add_node(id, node);
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

void ModulesGraph::add_link(Cool::LinkId const& id, Cool::Link const& link)
{
    _nodes_editor.graph().add_link(id, link);
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

void ModulesGraph::remove_node(Cool::NodeId const& id)
{
    _nodes_editor.graph().nodes().with_mutable_ref(id, [&](Cool::Node& node) {
        node.downcast<Node>().clear_all_error_messages();
    });
    _nodes_editor.graph().remove_node(id);
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

void ModulesGraph::remove_link(Cool::LinkId const& id)
{
    _nodes_editor.graph().remove_link(id);
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

auto ModulesGraph::try_get_node(Cool::NodeId const& id) const -> Node const*
{
    return graph().try_get_node<Node>(id);
}

void ModulesGraph::set_node(Cool::NodeId const& id, Node const& value)
{
    graph().nodes().with_mutable_ref(id, [&](Cool::Node& node) {
        node.downcast<Node>() = value;
    });
    rebuild_modules_graph_flag().set_dirty(); // Important when calling this function from a Command
}

} // namespace Lab
