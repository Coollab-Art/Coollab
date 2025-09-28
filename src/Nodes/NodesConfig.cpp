#include "NodesConfig.h"
#include <Commands/Command_AddLink.h>
#include <Commands/Command_AddNode.h>
#include <Commands/Command_ChangeNodeDefinition.h>
#include <Commands/Command_Meshing.h>
#include <Commands/Command_RemoveLink.h>
#include <Commands/Command_RemoveNode.h>
#include <Commands/Command_SetMainNodeId.h>
#include <Commands/Command_SetVariable.h>
#include <Commands/Command_SetVariableDefaultMetadata.h>
#include <Commands/Command_SetVariableDefaultValue.h>
#include <Commands/Command_SetVariableMetadata.h>
#include <Cool/Dependencies/always_requires_shader_code_generation.h>
#include <Cool/Nodes/ed.h>
#include <Nodes/NodesClipboard.h>
#include <algorithm>
#include <string>
#include "App.h"
#include "Cool/Audio/AudioManager.h"
#include "Cool/Dependencies/SharedVariable.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/Nodes/Link.h"
#include "Cool/Nodes/NodesLibrary.h"
#include "Cool/Nodes/Pin.h"
#include "Cool/Nodes/as_ed_id.h"
#include "Cool/String/String.h"
#include "Cool/StrongTypes/Color.h"
#include "Cool/Variables/Settings.h"
#include "Cool/Variables/Settings_Ref.h"
#include "Cool/Variables/Variable.h"
#include "FunctionSignature.h"
#include "Node.h"
#include "NodeColor.h"
#include "NodeDefinition.h"
#include "PrimitiveType.h"
#include "Serialization/SNodesClipboard.h"
#include "imgui.h"

namespace Lab {

// TODO(Settings) Remove
static auto settings_from_inputs(std::vector<Cool::AnySharedVariable> const& inputs) -> Cool::Settings
{
    auto settings = Cool::Settings{};
    settings.reserve(inputs.size());
    for (auto const& input : inputs)
    {
        std::visit([&](auto&& input) {
            settings.push_back(input.variable_data());
        },
                   input);
    }
    return settings;
}

// TODO(Settings) Remove
template<typename T>
static auto make_command_set_variable(Cool::AnyVariableData const& var_data, Cool::SharedVariable<T> const& var) -> Command_SetVariable<T>
{
    return Command_SetVariable<T>{
        .var_ref = var.get_ref(),
        .value   = std::get<Cool::VariableData<T>>(var_data).value,
    };
}
// TODO(Settings) Remove
template<typename T>
static auto make_command_set_variable_default_value(Cool::AnyVariableData const& var_data, Cool::SharedVariable<T> const& var) -> Command_SetVariableDefaultValue<T>
{
    return Command_SetVariableDefaultValue<T>{
        .var_ref       = var.get_ref(),
        .default_value = std::get<Cool::VariableData<T>>(var_data).value,
    };
}
// TODO(Settings) Remove
template<typename T>
static auto make_command_set_variable_metadata(Cool::AnyVariableData const& var_data, Cool::SharedVariable<T> const& var) -> Command_SetVariableMetadata<T>
{
    return Command_SetVariableMetadata<T>{
        .var_ref  = var.get_ref(),
        .metadata = std::get<Cool::VariableData<T>>(var_data).metadata,
    };
}
// TODO(Settings) Remove
template<typename T>
static auto make_command_set_variable_default_metadata(Cool::AnyVariableData const& var_data, Cool::SharedVariable<T> const& var) -> Command_SetVariableDefaultMetadata<T>
{
    return Command_SetVariableDefaultMetadata<T>{
        .var_ref          = var.get_ref(),
        .default_metadata = std::get<Cool::VariableData<T>>(var_data).metadata,
    };
}

// TODO(Settings) Remove
static void apply_settings_to_inputs(
    Cool::Settings const&                 settings,
    std::vector<Cool::AnySharedVariable>& inputs,
    std::string_view                      node_name,
    CommandExecutor const&                command_executor
)
{
    try
    {
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            std::visit([&](auto&& input) {
                command_executor.execute(make_command_set_variable(settings.at(i), input));
                command_executor.execute(make_command_set_variable_default_value(settings.at(i), input));
                command_executor.execute(make_command_set_variable_metadata(settings.at(i), input));
                command_executor.execute(make_command_set_variable_default_metadata(settings.at(i), input));
            },
                       inputs.at(i));
        }
    }
    catch (...)
    {
        // TODO(Settings) Remove this try-catch once we update presets properly
        Cool::Log::warning("Presets", fmt::format("Current preset for node \"{}\" does not match the INPUTs of the shader anymore, it has not been applied fully.", node_name));
    }
}

// TODO(Settings) Remove
template<typename T>
static auto get_concrete_variable_data(Cool::AnyVariableData const& var_data, Cool::SharedVariable<T> const&) -> Cool::VariableData<T>
{
    return std::get<Cool::VariableData<T>>(var_data);
}

// TODO(Settings) Remove
static void apply_settings_to_inputs_no_history(
    Cool::Settings const&                 settings,
    std::vector<Cool::AnySharedVariable>& inputs,
    std::string_view                      node_name
)
{
    try
    {
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            std::visit([&](auto&& input) {
                input.variable() = Cool::Variable{get_concrete_variable_data(settings.at(i), input)};
            },
                       inputs.at(i));
        }
    }
    catch (...)
    {
        // TODO(Settings) Remove this try-catch once we update presets properly
        Cool::Log::warning("Presets", fmt::format("Current preset for node \"{}\" does not match the INPUTs of the shader anymore, it has not been applied fully.", node_name));
    }
}

void NodesConfig::node_context_menu(Cool::Node& /*Node*/, Cool::NodeId const& /* node_id */)
{
    // if (ImGui::Button("Meshing"))
    // {
    //     _command_executor.execute(Command_Meshing{node_id});
    //     ImGui::CloseCurrentPopup();
    // }
}

auto NodesConfig::name(Cool::Node const& abstract_node) const -> std::string
{
    auto const& node = abstract_node.downcast<Node>();
    return node.name();
}

auto NodesConfig::category_name(Cool::Node const& abstract_node) const -> std::string
{
    auto const& node = abstract_node.downcast<Node>();
    return node.category_name();
}

void NodesConfig::main_node_toggle(Cool::NodeId const& node_id)
{
    bool is_main = node_id == _main_node_id;
    if (Cool::ImGuiExtras::toggle("Main node", &is_main))
        set_main_node_id(node_id);
}

void NodesConfig::imgui_above_node_pins(Cool::Node& /* abstract_node */, Cool::NodeId const& id)
{
    main_node_toggle(id);
}

void NodesConfig::imgui_below_node_pins(Cool::Node& /* abstract_node */, Cool::NodeId const& /* id */)
{
}

static auto value_input_is_connected_to_a_node(size_t value_input_index, Node const& node, Cool::NodesGraph const& graph) -> bool
{
    auto const  input_pin     = node.pin_of_value_input(value_input_index); // NOLINT(performance-unnecessary-copy-initialization)
    auto const  input_node_id = graph.find_node_connected_to_input_pin(input_pin.id());
    auto const* input_node    = graph.try_get_node<Node>(input_node_id);
    return input_node != nullptr;
}

void NodesConfig::imgui_in_inspector_above_node_info(Cool::Node& /* abstract_node */, Cool::NodeId const& /* id */)
{
    // auto& node = abstract_node.downcast<Node>();
    // main_node_toggle(id);
}

void NodesConfig::imgui_in_inspector_below_node_info(Cool::Node& abstract_node, Cool::NodeId const& /* id */)
{
    auto& node = abstract_node.downcast<Node>();

    if (node.is_audio_node())
    {
        if (Cool::ImGuiExtras::button_with_text_icon(ICOMOON_COG))
            _audio_manager.open_imgui_window();
        ImGui::SetItemTooltip("%s", "Open audio config to select which audio file to play, or which input device (e.g. microphone) to use.");
    }

    if (node.particles_count().has_value())
        ImGui::DragScalar("Particles Count", ImGuiDataType_U64, &node.particles_count().value());

    for (size_t i = 0; i < node.value_inputs().size(); ++i)
    {
        Cool::ImGuiExtras::disabled_if(
            value_input_is_connected_to_a_node(i, node, graph()),
            "This value is connected to a node in the graph, you cannot edit it here.", [&]() {
                _ui.widget(node.value_inputs()[i]);
            }
        );
    }

    auto const* def = _get_node_definition(node.id_names());
    if (!def)
    {
        ImGui::NewLine();
        ImGui::Separator();
        Cool::ImGuiExtras::warning_text("Node file not found");
    }
    else if (!node.value_inputs().empty())
    {
        ImGui::NewLine();
        ImGui::SeparatorText("Presets");
        // Get the variables from the inputs
        auto settings = settings_from_inputs(node.value_inputs());
        // Apply
        bool const has_changed = def->imgui_presets(settings);

        if (has_changed)
        {
            // Apply back the variables to the inputs' default variables
            apply_settings_to_inputs(settings, node.value_inputs(), to_string(node), _command_executor);
        }
    }
}

void NodesConfig::imgui_inspector_content_when_no_node_is_selected()
{
    if (_nodes_editor.graph().nodes().is_empty())
    {
        ImGui::PushFont(Cool::Font::italic());
        ImGui::TextUnformatted("Select a node to edit its parameters");
        ImGui::PopFont();
    }
    else
    {
        _command_executor.get_ctx().app().imgui_window_cameras();
    }
}

auto NodesConfig::node_color(Cool::Node const& abstract_node, Cool::NodeId const&) const -> Cool::Color
{
    auto const& node = abstract_node.downcast<Node>();
    auto const* def  = _get_node_definition(node.id_names());
    if (!def)
        return Cool::Color::from_srgb(glm::vec3{0.f});

    return compute_function_color(def->signature());
}

auto NodesConfig::pin_color(Cool::Pin const& pin, size_t pin_index, Cool::Node const& abstract_node, Cool::NodeId const& node_id) const -> Cool::Color
{
    if (pin.kind() == Cool::PinKind::Output)
    {
        // Main output pin
        if (pin_index == 0)
            return node_color(abstract_node, node_id);
        // Float output
        return NodeColor::greyscale();
    }

    auto const& node = abstract_node.downcast<Node>();
    auto const* def  = _get_node_definition(node.id_names());
    if (!def)
        return Cool::Color::from_srgb(glm::vec3{0.f});

    // For function inputs, use the color of the function's signature.
    if (node.function_input_pin_idx_begin() <= pin_index && pin_index < node.function_input_pin_idx_end())
    {
        return compute_function_color(def->function_inputs()[pin_index - node.function_input_pin_idx_begin()].signature());
    }
    // For value inputs, use the color corresponding to the Input type.
    if (node.value_input_pin_idx_begin() <= pin_index && pin_index < node.value_input_pin_idx_end())
    {
        return compute_value_input_color(def->value_inputs()[pin_index - node.value_input_pin_idx_begin()]);
    }
    // For main input pins, use the color corresponding to the `from` Primitive type.
    if (node.main_input_pin_idx_begin() <= pin_index && pin_index < node.main_input_pin_idx_end())
    {
        return compute_primitive_type_color(def->signature().from);
    }

    return NodeColor::miscellaneous();
}

void NodesConfig::on_node_created(Cool::Node& /* abstract_node */, Cool::NodeId const& node_id, Cool::Pin const* pin_linked_to_new_node)
{
    // Don't change main node if we are dragging a link backward.
    if (pin_linked_to_new_node && pin_linked_to_new_node->kind() == Cool::PinKind::Input)
        return;

    set_main_node_id(node_id);
}

void NodesConfig::set_main_node_id(Cool::NodeId const& id)
{
    _command_executor.execute(Command_SetMainNodeId{id});
}

void NodesConfig::on_link_created_between_existing_nodes(Cool::Link const& link, Cool::LinkId const&)
{
    // Traverse the graph until:
    // - We reach the end and set this as the main node
    // - We reach the main node and do nothing
    // - We reach the previous main_node and set this as the main node

    auto next_id = graph().find_node_containing_pin(link.to_pin_id);
    if (next_id == _main_node_id)
        return;
    auto const* next_node        = graph().try_get_node<Node>(next_id);
    auto        new_main_node_id = Cool::NodeId{};
    while (next_node)
    {
        new_main_node_id = next_id;
        next_id          = graph().find_node_connected_to_output_pin(next_node->output_pins()[0].id());
        if (next_id == _main_node_id)
            return;
        next_node = graph().try_get_node<Node>(next_id);
    }
    if (!new_main_node_id.underlying_uuid().is_nil())
        set_main_node_id(new_main_node_id);
}

static auto doesnt_need_main_pin(FunctionSignature const& signature) -> bool
{
    return
        // Image-like and shape 2D
        (signature.from == PrimitiveType::UV && signature.to != PrimitiveType::UV)
        // Curve
        || is_curve(signature)
        || is_curve_3D(signature)
        // Shape 3D
        || is_shape_3D(signature)
        // Particle nodes that are not Particle Modifiers
        || (signature.from == PrimitiveType::Particle2D && signature.to != PrimitiveType::Particle2D)
        || (signature.from == PrimitiveType::Particle3D && signature.to != PrimitiveType::Particle3D);
}

static auto wants_to_store_particles_count(NodeDefinition const& def) -> bool
{
    return Cool::String::contains(def.name(), "Init Particles Count"); // HACK to force only a node named "Init Particles Count" to display the particles count.
}

auto NodesConfig::add_node(Cool::NodeDefinitionAndCategoryName const& cat_id) -> Cool::NodeId
{
    return add_node(make_node(cat_id));
}

auto NodesConfig::add_node(Node const& node) -> Cool::NodeId
{
    auto const id = Cool::NodeId{reg::generate_uuid()};
    _command_executor.execute(Command_AddNode{id, node});
    return id;
}

auto NodesConfig::add_link(Cool::Link const& link) -> Cool::LinkId
{
    auto const id = Cool::LinkId{reg::generate_uuid()};
    _command_executor.execute(Command_AddLink{id, link});
    return id;
}

void NodesConfig::remove_node(Cool::NodeId const& id, Cool::Node const& node)
{
    _command_executor.execute(Command_RemoveNode{id, node.downcast<Node>()});
}

void NodesConfig::remove_link(Cool::LinkId const& id, Cool::Link const& link)
{
    _command_executor.execute(Command_RemoveLink{id, link});
}

static auto make_node(Cool::NodeDefinitionAndCategoryName const& cat_id, Cool::GetNodeCategoryConfig_Ref get_node_category_config, DirtyFlags const& dirty_flags) -> Node
{
    auto const def = cat_id.def.downcast<NodeDefinition>();

    bool const needs_main_pin = !doesnt_need_main_pin(def.signature());

    auto node = Node{
        {def.name(), cat_id.category_name},
        needs_main_pin ? def.signature().arity : 0,
        def.function_inputs().size(),
    };
    auto const node_category_config = get_node_category_config(cat_id.category_name);

    if (wants_to_store_particles_count(def))
        node.particles_count() = std::make_optional<size_t>(1000);

    if (needs_main_pin)
    {
        for (size_t i = 0; i < def.signature().arity; ++i)
        {
            node.input_pins().push_back(Cool::InputPin{/*pin_name=*/Cool::String::replace_all(def.main_function().argument_names[i], "_", " "), ""});
        }
    }
    node.output_pins().emplace_back("OUT", "");

    for (auto const& function_input : def.function_inputs())
        node.input_pins().push_back(Cool::InputPin{function_input.name(), function_input.description()});

    for (auto const& value_input_def : def.value_inputs())
    {
        node.value_inputs().push_back(std::visit(
            [&](auto&& value_input_def)
                -> Cool::AnySharedVariable { return Cool::SharedVariable{
                                                 value_input_def,
                                                 dirty_flags.primary(Cool::always_requires_shader_code_generation(value_input_def)),
                                                 dirty_flags.secondary()
                                             }; },
            value_input_def
        ));
        node.input_pins().push_back(std::visit([](auto&& value_input_def) { return Cool::InputPin{value_input_def.var_data.name, value_input_def.description.value_or("")}; }, value_input_def));
    }

    // Get the variables from the inputs
    auto settings = settings_from_inputs(node.value_inputs());
    // Apply
    def.presets_manager().apply_first_preset_if_there_is_one(settings);
    // Apply back the variables to the inputs' default variables
    apply_settings_to_inputs_no_history(settings, node.value_inputs(), to_string(node));

    for (auto const& output_index_name : def.output_indices())
        node.output_pins().push_back(Cool::OutputPin{output_index_name, ""});

    return node;
}

auto NodesConfig::make_node(Cool::NodeDefinitionAndCategoryName const& cat_id) -> Node
{
    return Lab::make_node(cat_id, _get_node_category_config, _dirty_flags);
}

struct PotentialLink {
    Cool::Link link;
    bool       from_copied_node{false};
    bool       to_copied_node{false};
};

auto NodesConfig::copy_nodes() const -> std::string
{
    auto clipboard       = NodesClipboard{};
    auto left_most_pos   = ImVec2{FLT_MAX, FLT_MAX};
    auto potential_links = std::vector<PotentialLink>{};

    _nodes_editor.for_each_selected_node([&](Cool::Node const& abstract_node, Cool::NodeId const& node_id) {
        auto const& node = abstract_node.downcast<Node>();

        auto const pos = ed::GetNodePosition(Cool::as_ed_id(node_id));
        if (pos.x < left_most_pos.x)
            left_most_pos = pos;

        clipboard.nodes.push_back({node.as_pod(), pos});

        _nodes_editor.graph().for_each_link_connected_to_node(abstract_node, [&](Cool::LinkId const&, Cool::Link const& link, bool is_connected_to_input_pin) {
            size_t index = potential_links.size();
            for (size_t i = 0; i < potential_links.size(); ++i)
            {
                if (potential_links[i].link == link)
                {
                    index = i;
                    break;
                }
            }
            if (index == potential_links.size())
                potential_links.push_back({link, false, false});

            if (is_connected_to_input_pin)
                potential_links[index].to_copied_node = true;
            else
                potential_links[index].from_copied_node = true;
        });
    });

    for (auto& node : clipboard.nodes)
        node.position -= left_most_pos;

    for (auto const& potential_link : potential_links)
    {
        if (!potential_link.to_copied_node) // Never copy links that are going into a node that is not copied. Otherwise we would need to remove the old link when pasting, because you cannot have two links going into the same node.
            continue;
        if (!potential_link.from_copied_node && !ImGui::GetIO().KeyShift) // Only copy links coming from a node not copied if SHIFT is pressed. Both options are valid and can be useful depending on the use case.
            continue;
        clipboard.links.push_back(potential_link.link);
    }

    return string_from_nodes_clipboard(clipboard);
}

auto NodesConfig::paste_nodes(std::string_view clipboard_string) -> bool
{
    try
    {
        auto              clipboard = string_to_nodes_clipboard(std::string{clipboard_string});
        std::vector<Node> new_nodes{};
        // Create all nodes but don't add them to the graph yet, because we need to call make_sure_node_uses_the_most_up_to_date_version_of_its_definition() first, which requires all the links to have been added to the graph. And before adding all the links we must first iterate over all the nodes and update the links with the new pin ids of the nodes.
        for (auto const& clipboard_node : clipboard.nodes)
        {
            auto node = Node{clipboard_node.node_as_pod.pod_part};
            for (auto const& value_input : clipboard_node.node_as_pod.value_inputs)
            {
                std::visit([&](auto&& value_input) {
                    node.value_inputs().push_back(Cool::SharedVariable{
                        value_input,
                        _dirty_flags.primary(Cool::always_requires_shader_code_generation(value_input)),
                        {}, // description, it will be set by make_sure_node_uses_the_most_up_to_date_version_of_its_definition()
                        _dirty_flags.secondary()
                    });
                },
                           value_input);
            }
            for (auto& pin : node.input_pins())
            {
                auto const new_pin_id = Cool::PinId{reg::generate_uuid()};
                for (auto& link : clipboard.links)
                {
                    if (link.to_pin_id == pin.id())
                        link.to_pin_id = new_pin_id;
                }
                pin.set_id(new_pin_id);
            }
            for (auto& pin : node.output_pins())
            {
                auto const new_pin_id = Cool::PinId{reg::generate_uuid()};
                for (auto& link : clipboard.links)
                {
                    if (link.from_pin_id == pin.id())
                        link.from_pin_id = new_pin_id;
                }
                pin.set_id(new_pin_id);
            }
            new_nodes.push_back(node);
        }

        for (auto const& link : clipboard.links)
            add_link(link);

        for (size_t i = 0; i < new_nodes.size(); ++i)
        {
            auto const& node_in_clipboard = clipboard.nodes[i];
            auto&       node              = new_nodes[i];

            // Check if the definition has changed (e.g. new inputs) and also finds description of variables if any.
            // Must be done after all the links have been added to the graph, because it might remove some that are outdated if some pins have been removed and they had a link connected to them.
            make_sure_node_uses_the_most_up_to_date_version_of_its_definition(node);

            auto const new_node_id    = add_node(node);
            auto const new_node_id_ed = Cool::as_ed_id(new_node_id);

            ed::SetNodePosition(new_node_id_ed, ImGui::GetMousePos() + node_in_clipboard.position);
            ed::SelectNode(new_node_id_ed, i > 0 /* keep_previously_selected_nodes */);
            // on_node_created(*graph().nodes().get_mutable_ref(new_node_id), new_node_id, nullptr); NB: we don't actually do that because we don't want to change the main_node_id
        }
        return true;
    }
    catch (std::exception const& e)
    {
        Cool::Log::internal_warning("Copy-Paste", fmt::format("Failed to paste nodes:\n{}", e.what()));
        return false;
    }
}

static auto name(Cool::AnySharedVariable const& var)
{
    return std::visit(([](auto&& var) { return var.name(); }), var);
}

static auto inputs_have_the_same_type_and_name(Cool::AnySharedVariable const& var1, Cool::AnySharedVariable const& var2) -> bool
{
    return var1.index() == var2.index()
           && name(var1) == name(var2);
}

static auto iterator_to_same_input(Cool::AnySharedVariable const& input, std::vector<Cool::AnySharedVariable>& old_inputs)
{
    return std::find_if(old_inputs.begin(), old_inputs.end(), [&](const Cool::AnySharedVariable& other_input) {
        return inputs_have_the_same_type_and_name(other_input, input);
    });
}

static void keep_values_of_inputs_that_already_existed_and_destroy_unused_ones(
    std::vector<Cool::AnySharedVariable>& old_inputs,
    std::vector<Cool::AnySharedVariable>& new_inputs
)
{
    for (auto& input : old_inputs)
    {
        auto const it = iterator_to_same_input(input, new_inputs);
        if (it != new_inputs.end())
        {
            auto       description         = std::visit([](auto&& input) { return std::move(input.description()); }, *it);      // Keep the new description
            auto const desired_color_space = std::visit([](auto&& input) { return input.get_ref().desired_color_space; }, *it); // Keep the new desired_color_space
            *it                            = std::move(input);
            std::visit([&](auto&& it) mutable { it.description() = std::move(description); }, *it);
            std::visit([&](auto&& it) { it.get_ref().desired_color_space = desired_color_space; }, *it);
        }
    }
}

template<typename PinT>
static void refresh_pins(std::vector<PinT>& new_pins, std::vector<PinT> const& old_pins, std::function<void(Cool::PinId const&)> const& remove_links)
{
    // Collect indices of pins that don't match any old pins.
    // Start with all the indices of the new input pins
    std::vector<size_t> indices(new_pins.size());
    std::iota(std::begin(indices), std::end(indices), 0u);
    // Remove indices of pins that have the same name as an old pin.
    std::erase_if(indices, [&](size_t idx) {
        return std::any_of(old_pins.begin(), old_pins.end(), [&](auto&& pin) {
            return pin.name() == new_pins[idx].name();
        });
    });
    // Make sure smallest indices are last because we read this vector from right to left.
    std::sort(indices.begin(), indices.end(), std::greater{});

    for (auto const& pin : old_pins)
    {
        // Try to find a pin with the same name
        auto const it = std::find_if(new_pins.begin(), new_pins.end(), [&](auto&& other_pin) {
            return other_pin.name() == pin.name();
        });
        if (it != new_pins.end())
        {
            it->set_id(pin.id());
        }
        // Otherwise default to one of the leftover indices
        else if (!indices.empty())
        {
            new_pins[indices.back()].set_id(pin.id());
            indices.pop_back();
        }
        else
        {
            remove_links(pin.id());
        }
    }
}

static void refresh_pins(Node& node, Node const& out_node, std::function<void(Cool::PinId const&)> const& remove_links)
{
    refresh_pins(node.input_pins(), out_node.input_pins(), remove_links);
    refresh_pins(node.output_pins(), out_node.output_pins(), remove_links);
}

void NodesConfig::change_node_definition(Cool::NodeId const& id, Cool::Node& node, Cool::NodeDefinition const& def)
{
    auto node_copy = node.downcast<Node>(); // Don't change the original node just yet, it will be done by the command
    update_node_with_new_definition(node_copy, def, true /*store_links_deletion_in_history*/);
    _command_executor.execute(Command_ChangeNodeDefinition{id, std::move(node_copy)});
}

void NodesConfig::update_node_with_new_definition(Cool::Node& abstract_out_node, Cool::NodeDefinition const& definition)
{
    update_node_with_new_definition(abstract_out_node.downcast<Node>(), definition);
}

static void update_node_with_new_definition(Node& out_node, Cool::NodeDefinition const& definition, Cool::NodesGraph& graph, CommandExecutor const& command_executor, Cool::GetNodeCategoryConfig_Ref get_node_category_config, DirtyFlags const& dirty_flags, bool store_links_deletion_in_history = false)
{
    auto node = make_node({definition, out_node.category_name()}, get_node_category_config, dirty_flags);
    node.set_name(out_node.name());
    if (node.particles_count().has_value() && out_node.particles_count().has_value())
        node.set_particles_count(out_node.particles_count());

    keep_values_of_inputs_that_already_existed_and_destroy_unused_ones(out_node.value_inputs(), node.value_inputs());

    if (store_links_deletion_in_history)
    {
        auto links_to_remove = std::vector<std::pair<Cool::LinkId, Cool::Link>>{};
        refresh_pins(node, out_node, [&](Cool::PinId const& pin_id) {
            graph.for_each_link_connected_to_pin(pin_id, [&](Cool::LinkId const& id, Cool::Link const& link, bool) { links_to_remove.emplace_back(id, link); });
        });
        for (auto const& [id, link] : links_to_remove)
            command_executor.execute(Command_RemoveLink{id, link});
    }
    else
    {
        refresh_pins(node, out_node, [&](Cool::PinId const& pin_id) { graph.remove_link_going_into(pin_id); });
    }

    out_node = std::move(node);
}

void NodesConfig::update_node_with_new_definition(Node& out_node, Cool::NodeDefinition const& definition, bool store_links_deletion_in_history)
{
    Lab::update_node_with_new_definition(out_node, definition, graph(), _command_executor, _get_node_category_config, _dirty_flags, store_links_deletion_in_history);
}

void make_sure_node_uses_the_most_up_to_date_version_of_its_definition(Node& node, Cool::GetNodeDefinition_Ref<NodeDefinition> get_node_definition, Cool::NodesGraph& graph, CommandExecutor const& command_executor, Cool::GetNodeCategoryConfig_Ref get_node_category_config, DirtyFlags const& dirty_flags)
{
    auto const* const node_def = get_node_definition(node.id_names());
    if (node_def)
        Lab::update_node_with_new_definition(node, *node_def, graph, command_executor, get_node_category_config, dirty_flags);
}

void NodesConfig::make_sure_node_uses_the_most_up_to_date_version_of_its_definition(Node& node)
{
    Lab::make_sure_node_uses_the_most_up_to_date_version_of_its_definition(node, _get_node_definition, graph(), _command_executor, _get_node_category_config, _dirty_flags);
}

auto NodesConfig::maybe_disable_node_definition() const -> Cool::MaybeDisableNodeDefinition
{
#if !defined(COOL_PARTICLES_DISABLED_REASON)
    return [](Cool::NodeDefinition const&, Cool::NodesCategory const&) -> std::optional<const char*> {
        return std::nullopt;
    };
#else
    return [](Cool::NodeDefinition const&, Cool::NodesCategory const& category) -> std::optional<const char*> {
        if (Cool::String::contains(category.name(), "Particle"))
            return COOL_PARTICLES_DISABLED_REASON;
        return std::nullopt;
    };
#endif
}

} // namespace Lab
