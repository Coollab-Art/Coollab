#include "ISFCodeGen.h"
#include <Cool/Expected/RETURN_IF_UNEXPECTED.h>
#include <Cool/Nodes/GetNodeDefinition_Ref.h>
#include <Cool/Nodes/NodeId.h>
#include <Cool/String/String.h>
#include <Cool/Variables/gen_input_shader_code.h>
#include <Nodes/PrimitiveType.h>
#include <fmt/core.h>
#include "Cool/Expected/RETURN_IF_ERROR.h"
#include "Cool/Nodes/Pin.h"
#include "Cool/StrongTypes/Angle.h"
#include "Cool/StrongTypes/Camera2D.h"
#include "Cool/StrongTypes/Color.h"
#include "Cool/StrongTypes/ColorAndAlpha.h"
#include "Cool/StrongTypes/ColorPalette.h"
#include "Cool/StrongTypes/Direction2D.h"
#include "Cool/StrongTypes/Gradient.h"
#include "Cool/StrongTypes/Hue.h"
#include "Cool/StrongTypes/MathExpression.h"
#include "Cool/StrongTypes/Point2D.h"
#include "Cool/TextureSource/TextureDescriptor.h"
#include "Cool/TextureSource/TextureSource_Image.h"
#include "Cool/Utils/overloaded.hpp"
#include "Nodes/CodeGen.h"
#include "Nodes/CodeGen_default_function.h"
#include "Nodes/CodeGen_implicit_conversion.h"
#include <Cool/Gpu/OpenGL/preprocess_shader_source.h>
#include "Nodes/Function.h"
#include "Nodes/FunctionSignature.h"
#include "Nodes/MaybeGenerateModule.h"
#include "Nodes/Node.h"
#include "Nodes/NodeDefinition.h"
#include "Nodes/gen_function_definition.h"
#include "Nodes/valid_glsl.h"
#include "Nodes/valid_input_name.h"
#include "Nodes/variable_to_primitive_type.h"

namespace Lab {

// ---- ISF-specific context that wraps CodeGenContext and adds ISF metadata ----

struct ISFContext {
    CodeGenContext           code_gen_context;
    ISFMetadata              metadata{};
    std::vector<std::string> errors{};
    std::vector<std::string> warnings{};
    std::vector<std::pair<std::filesystem::path, std::string>> files_to_copy{};
    std::vector<ISFCachePass> cache_passes{};
    int                      isf_input_counter{0}; // For unique naming
    std::string              connection_hint{};     // "NodeName_PinName" — set by caller before traversing a connected node, used to suffix ISF input names

    auto graph() const -> Cool::NodesGraph const& { return code_gen_context.graph(); }
    auto get_node_definition(Cool::NodeDefinitionIdentifier const& id) const -> NodeDefinition const*
    {
        return code_gen_context.get_node_definition(id);
    }

    /// Adds an ISF input only if one with the same name doesn't already exist.
    void add_input_if_new(ISFInput input)
    {
        for (auto const& existing : metadata.inputs)
        {
            if (existing.name == input.name)
                return;
        }
        metadata.inputs.push_back(std::move(input));
    }
};

// ---- Forward declarations ----

static auto isf_gen_desired_function(
    FunctionSignature desired_signature,
    Node const*       maybe_node,
    Cool::NodeId const& id,
    ISFContext&          context
) -> ExpectedFunctionName;

static auto isf_gen_desired_function(
    FunctionSignature               desired_signature,
    std::reference_wrapper<Node const> node,
    Cool::NodeId const&             id,
    ISFContext&                      context
) -> ExpectedFunctionName;

static auto isf_gen_desired_function(
    FunctionSignature     desired_signature,
    Cool::InputPin const& pin,
    ISFContext&            context,
    bool                  fallback_to_a_default_function = true
) -> ExpectedFunctionName;

static auto isf_gen_desired_function_implementation(
    FunctionSignature   current,
    FunctionSignature   desired,
    std::string_view    base_function_name,
    Node const&         node,
    Cool::NodeId const& node_id,
    ISFContext&          context
) -> tl::expected<std::string, std::string>;

// ---- Constant generation from SharedVariable values ----

static auto gen_constant_value(Cool::AnySharedVariable const& var) -> std::string
{
    auto const name = valid_input_name(var);
    return std::visit(
        Cool::overloaded{
            [&](Cool::SharedVariable<bool> const& v) -> std::string {
                return fmt::format("bool {} = {};", name, v.value() ? "true" : "false");
            },
            [&](Cool::SharedVariable<int> const& v) -> std::string {
                return fmt::format("int {} = {};", name, v.value());
            },
            [&](Cool::SharedVariable<float> const& v) -> std::string {
                return fmt::format("float {} = {};", name, v.value());
            },
            [&](Cool::SharedVariable<Cool::Point2D> const& v) -> std::string {
                return fmt::format("vec2 {} = vec2({}, {});", name, v.value().value.x, v.value().value.y);
            },
            [&](Cool::SharedVariable<glm::vec2> const& v) -> std::string {
                return fmt::format("vec2 {} = vec2({}, {});", name, v.value().x, v.value().y);
            },
            [&](Cool::SharedVariable<glm::vec3> const& v) -> std::string {
                return fmt::format("vec3 {} = vec3({}, {}, {});", name, v.value().x, v.value().y, v.value().z);
            },
            [&](Cool::SharedVariable<glm::vec4> const& v) -> std::string {
                return fmt::format("vec4 {} = vec4({}, {}, {}, {});", name, v.value().x, v.value().y, v.value().z, v.value().w);
            },
            [&](Cool::SharedVariable<Cool::Color> const& v) -> std::string {
                auto const srgb = v.value().as_sRGB();
                return fmt::format("vec3 {} = vec3({}, {}, {});", name, srgb.x, srgb.y, srgb.z);
            },
            [&](Cool::SharedVariable<Cool::ColorAndAlpha> const& v) -> std::string {
                auto const srgb = v.value().as_sRGB_StraightA();
                return fmt::format("vec4 {} = vec4({}, {}, {}, {});", name, srgb.x, srgb.y, srgb.z, srgb.w);
            },
            [&](Cool::SharedVariable<Cool::Angle> const& v) -> std::string {
                return fmt::format("float {} = {};", name, v.value().as_radians());
            },
            [&](Cool::SharedVariable<Cool::Direction2D> const& v) -> std::string {
                auto const dir = v.value().as_unit_vec2();
                return fmt::format("vec2 {} = vec2({}, {});", name, dir.x, dir.y);
            },
            [&](Cool::SharedVariable<Cool::Hue> const& v) -> std::string {
                return fmt::format("float {} = {};", name, v.value().from_0_to_1());
            },
            [&](Cool::SharedVariable<Cool::Camera2D> const& v) -> std::string {
                auto const m = v.value().transform_matrix();
                return fmt::format(
                    "mat3 {} = mat3({}, {}, {}, {}, {}, {}, {}, {}, {});",
                    name,
                    m[0][0], m[0][1], m[0][2],
                    m[1][0], m[1][1], m[1][2],
                    m[2][0], m[2][1], m[2][2]);
            },
            [&](Cool::SharedVariable<Cool::Camera> const&) -> std::string {
                return fmt::format("mat4 {} = mat4(1.0);", name);
            },
            [&](Cool::SharedVariable<glm::mat2> const& v) -> std::string {
                auto const& m = v.value();
                return fmt::format("mat2 {} = mat2({}, {}, {}, {});", name, m[0][0], m[0][1], m[1][0], m[1][1]);
            },
            [&](Cool::SharedVariable<glm::mat3> const& v) -> std::string {
                auto const& m = v.value();
                return fmt::format(
                    "mat3 {} = mat3({}, {}, {}, {}, {}, {}, {}, {}, {});",
                    name,
                    m[0][0], m[0][1], m[0][2],
                    m[1][0], m[1][1], m[1][2],
                    m[2][0], m[2][1], m[2][2]);
            },
            [&](Cool::SharedVariable<glm::mat4> const& v) -> std::string {
                auto const& m = v.value();
                return fmt::format(
                    "mat4 {} = mat4({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});",
                    name,
                    m[0][0], m[0][1], m[0][2], m[0][3],
                    m[1][0], m[1][1], m[1][2], m[1][3],
                    m[2][0], m[2][1], m[2][2], m[2][3],
                    m[3][0], m[3][1], m[3][2], m[3][3]);
            },
            [&](Cool::SharedVariable<Cool::TimeSpeed> const& v) -> std::string {
                return fmt::format("float {} = {};", name, v.value().value);
            },
            [&](Cool::SharedVariable<Cool::Time> const& v) -> std::string {
                return fmt::format("float {} = {};", name, v.value().as_seconds_double());
            },
            // These types generate functions, not simple uniforms — use the existing code gen.
            // They produce function definitions (not uniforms), so the name param is ignored here.
            [&](Cool::SharedVariable<Cool::Gradient> const& v) -> std::string {
                // The existing code generates `uniform GradientMark array[N];` which won't work in ISF.
                // For now, generate the same code — we'll need to post-process or accept this limitation.
                return Cool::gen_input_shader_code(v.value(), fmt::format("'{}'", v.name()));
            },
            [&](Cool::SharedVariable<Cool::ColorPalette> const& v) -> std::string {
                return Cool::gen_input_shader_code(v.value(), fmt::format("'{}'", v.name()));
            },
            [&](Cool::SharedVariable<Cool::MathExpression> const& v) -> std::string {
                return Cool::gen_input_shader_code(v.value(), fmt::format("'{}'", v.name()));
            },
            // These should only appear on special nodes (handled separately)
            [&](Cool::SharedVariable<Cool::MidiChannel> const&) -> std::string {
                return fmt::format("float {} = 0.0;", name);
            },
            [&](Cool::SharedVariable<Cool::OSCChannel> const&) -> std::string {
                return fmt::format("float {} = 0.0;", name);
            },
            [&](Cool::SharedVariable<Cool::TextureDescriptor_Image> const&) -> std::string {
                return "";
            },
            [&](Cool::SharedVariable<Cool::TextureDescriptor_Video> const&) -> std::string {
                return "";
            },
            [&](Cool::SharedVariable<Cool::TextureDescriptor_Webcam> const&) -> std::string {
                return "";
            },
            [&](Cool::SharedVariable<Cool::TextureDescriptor_SpoutSyphon> const&) -> std::string {
                return "";
            },
        },
        var);
}

// ---- Special node detection ----

static auto is_select_node(std::string const& name) -> bool
{
    return name == "Select"
           || name == "Multi-Select"
           || name == "MIDI Multi-Select"
           || name == "Select Randomly"
           || name == "Multi-Select with Transition"
           || name == "MIDI Multi-Select with Transition";
}

static auto is_feedback_loop(NodeDefinition const& def) -> bool
{
    return def.name() == "Feedback (One frame delay)";
}

static auto is_caching(NodeDefinition const& def) -> bool
{
    return def.name() == "Caching";
}

static auto is_jfa(NodeDefinition const& def) -> bool
{
    return def.name() == "Mask to Shape";
}

/// Returns true when the Select node should generate an ISF INPUT dropdown
/// for the selection index (i.e. the selector pin is NOT driven by a connection).
/// MIDI variants always need an ISF INPUT because MIDI built-ins don't exist in ISF.
/// Regular variants need it only when the selector pin is unconnected.
static auto select_needs_isf_input(
    Node const&             node,
    NodeDefinition const&   node_definition,
    Cool::NodesGraph const& graph) -> bool
{
    // MIDI variants always need an ISF INPUT (MIDI variables don't exist in ISF)
    if (node_definition.name().find("MIDI") != std::string::npos)
        return true;

    // For regular variants: check if the selector value input has a node connected.
    // If yes, the generic code gen path will traverse that connection properly.
    size_t property_index = 0;
    for (auto const& prop : node.value_inputs())
    {
        auto const name = std::visit([](auto&& v) { return v.name(); }, prop);
        if (name == "Use First Value" || name == "Selected ID" || name == "Random value")
        {
            auto const input_pin     = node.pin_of_value_input(property_index);
            auto const input_node_id = graph.find_node_connected_to_input_pin(input_pin.id());
            auto const maybe_node    = graph.try_get_node<Node>(input_node_id);
            if (maybe_node)
                return false; // Connected — let the generic path handle it
        }
        property_index++;
    }
    return true; // Not connected — need ISF INPUT dropdown
}

static auto sanitize_isf_name(std::string name) -> std::string
{
    for (auto& c : name)
    {
        if (!std::isalnum(c) && c != '_')
            c = '_';
    }
    return name;
}

// ---- Value inputs for ISF (emit constants instead of uniforms) ----

struct ISFValueInputs {
    std::string              code;
    std::vector<std::string> real_names;
};

static auto isf_gen_value_inputs(
    Node const& node,
    ISFContext&  context
) -> tl::expected<ISFValueInputs, std::string>
{
    ISFValueInputs res{};

    size_t property_index{0};
    for (auto const& prop : node.value_inputs())
    {
        auto const input_pin     = node.pin_of_value_input(property_index);
        auto       output_pin    = Cool::OutputPin{};
        auto const input_node_id = context.graph().find_node_connected_to_input_pin(input_pin.id(), &output_pin);
        auto const maybe_node    = context.graph().try_get_node<Node>(input_node_id);
        if (maybe_node)
        {
            if (maybe_node->main_output_pin() == output_pin)
            {
                auto const property_type = variable_to_primitive_type(prop);
                if (!property_type)
                    return tl::make_unexpected("Can't create an INPUT with that type");

                auto const prop_name       = std::visit([](auto&& v) { return v.name(); }, prop);
                auto const prev_hint       = context.connection_hint;
                context.connection_hint     = sanitize_isf_name(fmt::format("{}_{}", node.definition_name(), prop_name));
                auto const input_func_name = isf_gen_desired_function(
                    {.from = PrimitiveType::Void, .to = *property_type, .arity = 0},
                    *maybe_node,
                    input_node_id,
                    context
                );
                context.connection_hint = prev_hint;
                RETURN_IF_UNEXPECTED(input_func_name);
                res.real_names.push_back(fmt::format("{}()", *input_func_name));
            }
            else
            {
                res.real_names.push_back(make_valid_output_index_name(output_pin));
            }
        }
        else
        {
            // No node connected: emit constant instead of uniform
            res.code += gen_constant_value(prop) + '\n';
            res.real_names.push_back(valid_input_name(prop));
        }

        property_index++;
    }

    return res;
}

// ---- Function inputs (recursive traversal) ----

struct ISFGeneratedInputs {
    std::unordered_map<std::string, std::string> real_names;
};

static auto isf_gen_function_inputs(
    Node const&           node,
    NodeDefinition const& node_definition,
    ISFContext&            context
) -> tl::expected<ISFGeneratedInputs, std::string>
{
    ISFGeneratedInputs res;

    for (size_t fn_input_idx = 0; fn_input_idx < node_definition.function_inputs().size(); ++fn_input_idx)
    {
        auto const& fn_input      = node_definition.function_inputs()[fn_input_idx];
        auto        output_pin    = Cool::OutputPin{};
        auto const  input_node_id = context.graph().find_node_connected_to_input_pin(node.pin_of_function_input(fn_input_idx).id(), &output_pin);
        auto const  input_node    = context.graph().try_get_node<Node>(input_node_id);
        if (!input_node || output_pin == input_node->main_output_pin())
        {
            auto const prev_hint   = context.connection_hint;
            context.connection_hint = sanitize_isf_name(fmt::format("{}_{}", node_definition.name(), fn_input.name()));
            auto const func_name   = isf_gen_desired_function(
                fn_input.signature(),
                input_node,
                input_node_id,
                context
            );
            context.connection_hint = prev_hint;
            RETURN_IF_UNEXPECTED(func_name);
            res.real_names[fn_input.name()] = *func_name;
        }
        else
        {
            res.real_names[fn_input.name()] = make_valid_output_index_name(output_pin);
        }
    }

    return res;
}

// ---- Base function generation (mirrors gen_base_function from CodeGen.cpp) ----

static auto base_function_name(NodeDefinition const& definition, Cool::NodeId const& id) -> std::string
{
    return valid_glsl(fmt::format("{}{}", definition.name(), to_string(id.underlying_uuid())));
}

static auto desired_function_name(NodeDefinition const& definition, Cool::NodeId const& id, FunctionSignature signature) -> std::string
{
    return valid_glsl(fmt::format("{}{}{}", definition.name(), to_string(signature), to_string(id.underlying_uuid())));
}

static auto isf_gen_base_function(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const function_inputs = isf_gen_function_inputs(node, node_definition, context);
    RETURN_IF_UNEXPECTED(function_inputs);

    auto const value_inputs = isf_gen_value_inputs(node, context);
    RETURN_IF_UNEXPECTED(value_inputs);

    auto const func_name = base_function_name(node_definition, id);

    // Preprocess helper GLSL to resolve #include directives (e.g. hexagonal_grid.glsl for Dithered Blur).
    // Without this, strip_include_lines() in ISFExport.cpp would remove the raw #include lines,
    // leaving functions like Cool_hex_uv undefined.
    auto helper_glsl = node_definition.helper_glsl_code();
    if (helper_glsl.find("#include") != std::string::npos)
    {
        auto preprocessed = Cool::OpenGL::preprocess_shader_source(helper_glsl);
        if (preprocessed)
            helper_glsl = *preprocessed;
    }

    auto func_implementation = gen_function_definition({
        .signature_as_string = node_definition.main_function().signature_as_string,
        .unique_name         = func_name,
        .before_function     = value_inputs->code + helper_glsl,
        .body                = node_definition.main_function().body,
    });

    // Replace input names with their generated counterparts
    {
        size_t i{0};
        for (auto const& value_input : node.value_inputs())
        {
            std::visit([&](auto&& value_input) {
                Cool::String::replace_all_inplace(func_implementation, fmt::format("'{}'", value_input.name()), value_inputs->real_names[i]);
            },
                       value_input);
            i++;
        }
    }

    // Replace function input names
    for (auto const& [old_name, new_name] : function_inputs->real_names)
        Cool::String::replace_all_inplace(func_implementation, fmt::format("'{}'", old_name), new_name);

    // Replace output index names
    for (size_t i = 1; i < node.output_pins().size(); ++i)
    {
        Cool::String::replace_all_inplace(
            func_implementation,
            fmt::format("'{}'", node.output_pins()[i].name()),
            make_valid_output_index_name(node.output_pins()[i]));
    }

    // Namespace global scope names
    for (auto const& name : node_definition.names_in_global_scope())
        Cool::String::replace_all_words_inplace(func_implementation, name, valid_glsl(fmt::format("{}{}", name, reg::to_string(id))));

    // Check no single quotes remain
    {
        auto const pos = func_implementation.find('\'');
        if (pos != std::string_view::npos)
        {
            auto const pos2 = func_implementation.find('\'', pos + 1);
            if (pos2 != std::string_view::npos)
                return tl::make_unexpected(fmt::format("Unresolved input name: \"{}\"", Cool::String::substring(func_implementation, pos, pos2 + 1)));
        }
    }

    context.code_gen_context.push_function({.name = func_name, .definition = func_implementation});
    return func_name;
}

// ---- Special node handlers ----

static auto make_isf_input_name(std::string const& base_name, ISFContext& context) -> std::string
{
    auto name = base_name;
    if (!context.connection_hint.empty())
        name += "_" + context.connection_hint;
    return sanitize_isf_name(name);
}

/// Handles Input Number, Input Integer, Input Angle, etc.
/// Returns the ISF input name (which ISF auto-provides as a uniform).
static auto handle_input_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // The node has exactly one value input which should become an ISF INPUT
    if (node.value_inputs().empty())
        return tl::make_unexpected(fmt::format("Input node '{}' has no value inputs", node_definition.name()));

    auto const& prop = node.value_inputs()[0];
    auto const isf_name = make_isf_input_name(
        std::visit([](auto&& v) { return v.name(); }, prop),
        context);

    // Create the ISF INPUT based on node type
    if (node_definition.name() == "Input Number")
    {
        auto const& v = std::get<Cool::SharedVariable<float>>(prop);
        context.add_input_if_new({isf_name, ISFInputFloat{v.value(), {}, {}}});
    }
    else if (node_definition.name() == "Input Integer")
    {
        // ISF's "long" type is for enums (dropdowns), not integer sliders.
        // Using float is the closest ISF equivalent for a numeric integer input.
        auto const& v = std::get<Cool::SharedVariable<int>>(prop);
        context.add_input_if_new({isf_name, ISFInputFloat{static_cast<double>(v.value()), {}, {}}});
    }
    else if (node_definition.name() == "Input Angle")
    {
        auto const& v = std::get<Cool::SharedVariable<Cool::Angle>>(prop);
        context.add_input_if_new({isf_name, ISFInputFloat{v.value().as_radians(), 0.0, 6.283185307}});
    }
    else if (node_definition.name() == "Input Point 2D")
    {
        auto const& v = std::get<Cool::SharedVariable<Cool::Point2D>>(prop);
        context.add_input_if_new({isf_name, ISFInputPoint2D{std::array{static_cast<double>(v.value().value.x), static_cast<double>(v.value().value.y)}, {}, {}}});
    }
    else if (node_definition.name() == "Input Color")
    {
        auto const& v   = std::get<Cool::SharedVariable<Cool::Color>>(prop);
        auto const  rgb = v.value().as_sRGB();
        context.add_input_if_new({isf_name, ISFInputColor{std::array{static_cast<double>(rgb.x), static_cast<double>(rgb.y), static_cast<double>(rgb.z), 1.0}}});
    }
    else if (node_definition.name() == "Input Vec2")
    {
        auto const& v = std::get<Cool::SharedVariable<glm::vec2>>(prop);
        context.add_input_if_new({isf_name, ISFInputPoint2D{std::array{static_cast<double>(v.value().x), static_cast<double>(v.value().y)}, {}, {}}});
    }
    else if (node_definition.name() == "Input Bool" || node_definition.name() == "Input Boolean")
    {
        auto const& v = std::get<Cool::SharedVariable<bool>>(prop);
        context.add_input_if_new({isf_name, ISFInputBool{v.value()}});
    }
    else
    {
        // Generic input: add as float with current value as default
        context.add_input_if_new({isf_name, ISFInputFloat{}});
    }

    // Generate a function that simply returns the ISF-provided uniform
    auto const func_name = base_function_name(node_definition, id);
    auto const ret_type  = node_definition.main_function().signature_as_string.return_type;
    auto const func_def  = fmt::format(
        R"glsl(
{} {}/*needs_coollab_context*/()
{{
    return {};
}}
)glsl",
        ret_type, func_name, isf_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

/// Checks if a node is connected to the value input at `value_input_index`.
/// If connected, traverses it via isf_gen_desired_function and returns "func_name()" as a GLSL expression.
/// If not connected, returns std::nullopt so the caller can use the hardcoded value instead.
static auto try_gen_connected_value_input(
    Node const&         node,
    size_t              value_input_index,
    PrimitiveType       expected_type,
    std::string const&  parent_node_name,
    std::string const&  pin_name,
    ISFContext&          context
) -> std::optional<std::string>
{
    auto const  input_pin     = node.pin_of_value_input(value_input_index);
    auto        output_pin    = Cool::OutputPin{};
    auto const  input_node_id = context.graph().find_node_connected_to_input_pin(input_pin.id(), &output_pin);
    auto const* maybe_node    = context.graph().try_get_node<Node>(input_node_id);
    if (!maybe_node || maybe_node->main_output_pin() != output_pin)
        return std::nullopt;

    auto const prev_hint   = context.connection_hint;
    context.connection_hint = sanitize_isf_name(fmt::format("{}_{}", parent_node_name, pin_name));
    auto const func_name   = isf_gen_desired_function(
        {.from = PrimitiveType::Void, .to = expected_type, .arity = 0},
        *maybe_node,
        input_node_id,
        context
    );
    context.connection_hint = prev_hint;
    if (!func_name)
        return std::nullopt;

    return fmt::format("{}()", *func_name);
}

static auto handle_time_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // Time node has Speed and Offset value inputs
    // Generate: TIME * speed + offset
    // Check if nodes are connected to the value input pins (e.g. MIDI → Speed)
    auto speed_str  = std::string{"1.0"};
    auto offset_str = std::string{"0.0"};

    size_t value_input_index = 0;
    for (auto const& prop : node.value_inputs())
    {
        auto const name = std::visit([](auto&& v) { return v.name(); }, prop);
        if (name == "Speed")
        {
            if (auto connected = try_gen_connected_value_input(node, value_input_index, PrimitiveType::Float, "Time", "Speed", context))
                speed_str = *connected;
            else if (auto const* v = std::get_if<Cool::SharedVariable<Cool::TimeSpeed>>(&prop))
                speed_str = fmt::format("{}", v->value().value);
        }
        else if (name == "Offset")
        {
            if (auto connected = try_gen_connected_value_input(node, value_input_index, PrimitiveType::Float, "Time", "Offset", context))
                offset_str = *connected;
            else if (auto const* v = std::get_if<Cool::SharedVariable<Cool::Time>>(&prop))
                offset_str = fmt::format("{}", v->value().as_seconds_double());
        }
        value_input_index++;
    }

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
float {}/*needs_coollab_context*/()
{{
    return TIME * {} + {};
}}
)glsl",
        func_name, speed_str, offset_str);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_volume_audio_node(
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const isf_name = std::string{"audio_volume"};
    context.add_input_if_new({isf_name, ISFInputAudio{1}}); // MAX=1 convention for volume

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
float {}/*needs_coollab_context*/()
{{
    return IMG_NORM_PIXEL({}, vec2(0.0)).r;
}}
)glsl",
        func_name, isf_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

/// Generates an audio texture sampling function that directly matches `desired_signature`.
/// This avoids the generic conversion machinery (isf_gen_desired_function_implementation)
/// which has a type mismatch bug when converting UV→Float inputs through naga's strict GLSL.
static auto gen_audio_sampler_with_desired_signature(
    std::string const&  isf_name,
    std::string const&  stretch_str,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    FunctionSignature     desired_signature,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const func_name = desired_function_name(node_definition, id, desired_signature);
    auto const return_type = glsl_type_as_string(desired_signature.to);

    // Build parameter list
    std::string params;
    for (size_t i = 0; i < desired_signature.arity; ++i)
    {
        if (i > 0) params += ", ";
        params += fmt::format("{} in{}", glsl_type_as_string(desired_signature.from), i);
    }

    // Convert input to a float sample position
    // If arity > 0, convert the first input to float; otherwise use 0.5 as default
    std::string sample_expr;
    if (desired_signature.arity > 0)
    {
        auto input_conversion = gen_implicit_conversion(desired_signature.from, PrimitiveType::Float, context.code_gen_context);
        if (input_conversion && !input_conversion->empty())
            sample_expr = fmt::format("{}(in0)", *input_conversion);
        else if (desired_signature.from == PrimitiveType::UV || desired_signature.from == PrimitiveType::Vec2)
            sample_expr = "in0.x * 0.5 / _aspect_ratio + 0.5"; // default UV→Float
        else
            sample_expr = "in0";
    }
    else
    {
        sample_expr = "0.5";
    }

    // Sample the audio texture
    std::string raw_value = fmt::format("IMG_NORM_PIXEL({}, vec2({} / {}, 0.0)).r", isf_name, sample_expr, stretch_str);

    // Convert float result to desired output type
    auto output_conversion = gen_implicit_conversion(PrimitiveType::Float, desired_signature.to, context.code_gen_context);
    std::string result_expr;
    if (output_conversion && !output_conversion->empty())
        result_expr = fmt::format("{}({})", *output_conversion, raw_value);
    else
        result_expr = raw_value;

    auto const func_def = fmt::format(
        R"glsl(
{} {}/*needs_coollab_context*/({})
{{
    return {};
}}
)glsl",
        return_type, func_name, params, result_expr);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_spectrum_audio_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    FunctionSignature     desired_signature,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const isf_name = std::string{"audio_fft"};
    context.add_input_if_new({isf_name, ISFInputAudioFFT{}});

    auto stretch_str = std::string{"1.0"};
    for (auto const& prop : node.value_inputs())
    {
        auto const name = std::visit([](auto&& v) { return v.name(); }, prop);
        if (name == "Stretch")
        {
            if (auto const* v = std::get_if<Cool::SharedVariable<float>>(&prop))
                stretch_str = fmt::format("{}", v->value());
        }
    }

    return gen_audio_sampler_with_desired_signature(isf_name, stretch_str, node_definition, id, desired_signature, context);
}

static auto handle_waveform_audio_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    FunctionSignature     desired_signature,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const isf_name = std::string{"audio_waveform"};
    context.add_input_if_new({isf_name, ISFInputAudio{}});

    auto stretch_str = std::string{"1.0"};
    for (auto const& prop : node.value_inputs())
    {
        auto const name = std::visit([](auto&& v) { return v.name(); }, prop);
        if (name == "Stretch")
        {
            if (auto const* v = std::get_if<Cool::SharedVariable<float>>(&prop))
                stretch_str = fmt::format("{}", v->value());
        }
    }

    return gen_audio_sampler_with_desired_signature(isf_name, stretch_str, node_definition, id, desired_signature, context);
}

static auto handle_image_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // Find the image path from the TextureDescriptor_Image value input
    std::string image_path_str;
    for (auto const& prop : node.value_inputs())
    {
        if (auto const* v = std::get_if<Cool::SharedVariable<Cool::TextureDescriptor_Image>>(&prop))
        {
            if (auto const* img_source = std::get_if<Cool::TextureSource_Image>(&v->value().source))
                image_path_str = img_source->absolute_path.string();
            break;
        }
    }

    // Deduplicate: reuse existing import if the same source file was already imported
    std::string isf_name;
    if (!image_path_str.empty())
    {
        for (auto const& [existing_source, existing_dest] : context.files_to_copy)
        {
            if (existing_source.string() == image_path_str)
            {
                for (auto const& [name, path] : context.metadata.imported)
                {
                    if (path == existing_dest)
                    {
                        isf_name = name;
                        break;
                    }
                }
                break;
            }
        }
    }

    if (isf_name.empty())
    {
        isf_name = fmt::format("imported_image_{}", context.isf_input_counter++);
        if (!image_path_str.empty())
        {
            auto const filename = std::filesystem::path{image_path_str}.filename().string();
            context.metadata.imported[isf_name] = filename;
            context.files_to_copy.push_back({image_path_str, filename});
        }
        else
        {
            // No image set, create as image INPUT so user can choose in v2
            context.add_input_if_new({isf_name, ISFInputImage{}});
        }
    }

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
vec4 {}/*needs_coollab_context*/(vec2 uv)
{{
    uv /= 2.;
    uv.x /= (IMG_SIZE({}).x / IMG_SIZE({}).y);
    uv += 0.5;
    return IMG_NORM_PIXEL({}, uv);
}}
)glsl",
        func_name, isf_name, isf_name, isf_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_webcam_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto const isf_name = std::string{"webcam_input"};
    context.add_input_if_new({isf_name, ISFInputImage{}});

    // Check for Mirror input
    auto mirror = false;
    for (auto const& prop : node.value_inputs())
    {
        if (auto const* v = std::get_if<Cool::SharedVariable<bool>>(&prop))
        {
            if (v->name() == "Mirror")
                mirror = v->value();
        }
    }

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
vec4 {}/*needs_coollab_context*/(vec2 uv)
{{
    {}
    uv /= 2.;
    uv.x /= (IMG_SIZE({}).x / IMG_SIZE({}).y);
    uv += 0.5;
    return IMG_NORM_PIXEL({}, uv);
}}
)glsl",
        func_name,
        mirror ? "uv.x *= -1.;" : "",
        isf_name, isf_name, isf_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_midi_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // Midi node: INPUT Midi 'Channel'; INPUT float 'Min'; INPUT float 'Max';
    // Generate an ISF INPUT float for the MIDI channel
    auto channel_name = std::string{"midi_input"};
    auto min_val      = 0.0;
    auto max_val      = 1.0;

    for (auto const& prop : node.value_inputs())
    {
        auto const name = std::visit([](auto&& v) { return v.name(); }, prop);
        if (name == "Channel")
        {
            if (auto const* v = std::get_if<Cool::SharedVariable<Cool::MidiChannel>>(&prop))
                channel_name = make_isf_input_name(fmt::format("midi_{}", v->value().index), context);
        }
        else if (name == "Min")
        {
            if (auto const* v = std::get_if<Cool::SharedVariable<float>>(&prop))
                min_val = v->value();
        }
        else if (name == "Max")
        {
            if (auto const* v = std::get_if<Cool::SharedVariable<float>>(&prop))
                max_val = v->value();
        }
    }

    auto const default_val = (min_val + max_val) / 2.0;
    context.add_input_if_new({channel_name, ISFInputFloat{default_val, min_val, max_val}});

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
float {}/*needs_coollab_context*/()
{{
    return {};
}}
)glsl",
        func_name, channel_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_osc_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    auto channel_name = std::string{"osc_input"};

    for (auto const& prop : node.value_inputs())
    {
        if (auto const* v = std::get_if<Cool::SharedVariable<Cool::OSCChannel>>(&prop))
            channel_name = make_isf_input_name(fmt::format("osc_{}", v->value().name), context);
    }

    context.add_input_if_new({channel_name, ISFInputFloat{0.0, 0.0, 1.0}});

    auto const func_name = base_function_name(node_definition, id);
    auto const func_def  = fmt::format(
        R"glsl(
float {}/*needs_coollab_context*/()
{{
    return {};
}}
)glsl",
        func_name, channel_name);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_select_node(
    Node const&           node,
    NodeDefinition const& node_definition,
    Cool::NodeId const&   id,
    FunctionSignature     desired_signature,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // Select nodes (MIDI Multi-Select, Select, etc.): create an ISF long INPUT
    // for the selection index, and generate GLSL that picks the right image.
    auto const isf_name = make_isf_input_name("selection", context);

    // Find how many function inputs are actually connected
    struct ConnectedImage {
        std::string func_name;
        int         index;
    };
    std::vector<ConnectedImage> connected_images;

    for (size_t i = 0; i < node_definition.function_inputs().size(); ++i)
    {
        auto const& fn_input     = node_definition.function_inputs()[i];
        auto        output_pin   = Cool::OutputPin{};
        auto const  input_node_id = context.graph().find_node_connected_to_input_pin(node.pin_of_function_input(i).id(), &output_pin);
        auto const  input_node    = context.graph().try_get_node<Node>(input_node_id);
        if (input_node && output_pin == input_node->main_output_pin())
        {
            auto const func_name = isf_gen_desired_function(
                desired_signature,
                *input_node,
                input_node_id,
                context
            );
            if (func_name)
                connected_images.push_back({*func_name, static_cast<int>(i)});
        }
    }

    if (connected_images.empty())
        return gen_default_function(desired_signature, context.code_gen_context);

    // Create ISF long input with labels
    ISFInputLong long_input;
    long_input.default_val = 0;
    for (size_t i = 0; i < connected_images.size(); ++i)
    {
        long_input.values.push_back(static_cast<int>(i));
        long_input.labels.push_back(fmt::format("Option {}", i + 1));
    }
    context.add_input_if_new({isf_name, long_input});

    // Generate GLSL function that selects based on the ISF input
    auto const func_name = base_function_name(node_definition, id);

    auto const return_type = glsl_type_as_string(desired_signature.to);
    auto const arity       = desired_signature.arity;
    auto const param_type  = glsl_type_as_string(desired_signature.from);

    std::string args_decl;
    std::string args_call;
    for (size_t i = 0; i < arity; ++i)
    {
        if (i > 0)
        {
            args_decl += ", ";
            args_call += ", ";
        }
        args_decl += fmt::format("{} in{}", param_type, i);
        args_call += fmt::format("in{}", i);
    }

    std::string body;
    for (size_t i = 0; i < connected_images.size(); ++i)
    {
        if (i == 0)
            body += fmt::format("    if ({} == {}) return {}({});\n", isf_name, i, connected_images[i].func_name, args_call);
        else if (i == connected_images.size() - 1)
            body += fmt::format("    else return {}({});\n", connected_images[i].func_name, args_call);
        else
            body += fmt::format("    else if ({} == {}) return {}({});\n", isf_name, i, connected_images[i].func_name, args_call);
    }

    auto const needs_context = "/*needs_coollab_context*/";
    auto const func_def = fmt::format(
        R"glsl(
{} {}{}({})
{{
{}}}
)glsl",
        return_type, func_name, needs_context,
        args_decl.empty() ? "" : args_decl,
        body);

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

static auto handle_caching_node(
    Node const&           node,
    NodeDefinition const& /*node_definition*/,
    Cool::NodeId const&   id,
    ISFContext&            context
) -> ExpectedFunctionName
{
    // Each Caching node becomes an ISF render pass: the sub-graph is rendered
    // to a TARGET texture, and consumers read from it via IMG_NORM_PIXEL.
    // The ISF runtime executes passes in order (PASSINDEX 0, 1, …), so by the
    // time the final pass runs all cached textures are ready.

    if (node.input_pins().empty())
        return tl::make_unexpected("Caching node has no input connected");

    auto const predecessor_id   = context.graph().find_node_connected_to_input_pin(node.input_pins()[0].id());
    auto const predecessor_node = context.graph().try_get_node<Node>(predecessor_id);
    if (!predecessor_node)
        return tl::make_unexpected("Caching node has no input connected");

    auto const pred_func = isf_gen_desired_function(
        {.from = PrimitiveType::UV, .to = PrimitiveType::sRGB_StraightA, .arity = 1},
        *predecessor_node,
        predecessor_id,
        context
    );
    if (!pred_func)
        return tl::make_unexpected(pred_func.error());

    auto const target_name = fmt::format("cache_{}", context.cache_passes.size());
    context.cache_passes.push_back({target_name, *pred_func});
    context.metadata.passes.push_back({target_name, false});

    // Return a function that reads from the cached pass texture
    auto const read_func_name = fmt::format("read_{}", target_name);
    auto const func_def       = fmt::format(
        R"glsl(
vec4 {}/*needs_coollab_context*/(vec2 uv)
{{
    return IMG_NORM_PIXEL({}, unnormalize_uv(uv));
}}
)glsl",
        read_func_name, target_name);

    context.code_gen_context.push_function({.name = read_func_name, .definition = func_def});
    return read_func_name;
}

// ---- ISF version of gen_desired_function_implementation (copied from CodeGen_desired_function_implementation.cpp) ----
// All gen_desired_function calls redirected to isf_gen_desired_function so that
// nodes reached through type-conversion wrappers go through the ISF path (producing
// ISF inputs / constants) instead of the original path (which produces uniforms).

namespace {

class ISF_TransformationStrategy_DoNothing {
public:
    static auto gen_func(Cool::InputPin const&, ISFContext&)
        -> ExpectedFunctionName
    {
        return "";
    }
};

class ISF_TransformationStrategy_UseDefaultFunction {
public:
    auto gen_func(Cool::InputPin const&, ISFContext& context) const
        -> ExpectedFunctionName
    {
        return gen_default_function(
            _signature,
            context.code_gen_context
        );
    }

    explicit ISF_TransformationStrategy_UseDefaultFunction(FunctionSignature signature)
        : _signature{signature} {}

private:
    FunctionSignature _signature;
};

class ISF_TransformationStrategy_UseInputNode {
public:
    auto gen_func(Cool::InputPin const& pin, ISFContext& context) const
        -> ExpectedFunctionName
    {
        return isf_gen_desired_function(
            _signature,
            pin,
            context
        );
    }

    explicit ISF_TransformationStrategy_UseInputNode(FunctionSignature signature)
        : _signature{signature} {}

private:
    FunctionSignature _signature;
};

class ISF_TransformationStrategy_UseInputNodeIfItExists {
public:
    auto gen_func(Cool::InputPin const& pin, ISFContext& context) const
        -> ExpectedFunctionName
    {
        return isf_gen_desired_function(
            _signature,
            pin,
            context,
            false /*fallback_to_a_default_function*/
        );
    }

    explicit ISF_TransformationStrategy_UseInputNodeIfItExists(FunctionSignature signature)
        : _signature{signature} {}

private:
    FunctionSignature _signature;
};

class ISF_TransformationStrategy {
public:
    using Variant = std::variant<
        ISF_TransformationStrategy_DoNothing,
        ISF_TransformationStrategy_UseDefaultFunction,
        ISF_TransformationStrategy_UseInputNode,
        ISF_TransformationStrategy_UseInputNodeIfItExists>;

    ISF_TransformationStrategy(Variant strategy) // NOLINT (google-explicit-constructor)
        : _strategy{strategy}
    {}

    auto gen_func(Cool::InputPin const& pin, ISFContext& context) const
        -> ExpectedFunctionName
    {
        return std::visit([&](auto&& strategy) { return strategy.gen_func(pin, context); }, _strategy);
    }

private:
    Variant _strategy;
};

} // namespace

static auto isf_input_transformation(
    FunctionSignature          current,
    FunctionSignature          desired,
    ImplicitConversions const& implicit_conversions
) -> ISF_TransformationStrategy
{
    auto const signature = FunctionSignature{
        .from  = desired.from,
        .to    = current.from,
        .arity = desired.from != PrimitiveType::Void ? 1u : 0u,
    };

    if (!implicit_conversions.input)
        return {ISF_TransformationStrategy_UseInputNode{signature}};

    if (implicit_conversions.output)
        return {ISF_TransformationStrategy_UseInputNodeIfItExists{signature}};

    return {ISF_TransformationStrategy_DoNothing{}};
}

static auto isf_output_transformation(
    FunctionSignature          current,
    FunctionSignature          desired,
    ImplicitConversions const& implicit_conversions
) -> ISF_TransformationStrategy
{
    if (implicit_conversions.output)
        return {ISF_TransformationStrategy_DoNothing{}};

    auto const signature = FunctionSignature{
        .from  = current.to,
        .to    = desired.to,
        .arity = current.to != PrimitiveType::Void ? 1u : 0u,
    };

    if (!implicit_conversions.input)
        return {ISF_TransformationStrategy_UseDefaultFunction{signature}};

    return {ISF_TransformationStrategy_UseInputNode{signature}};
}

static auto isf_argument_name(size_t i, size_t desired_arity)
    -> std::string
{
    if (desired_arity == 0)
        return "";

    return fmt::format("in{}", std::min(i, desired_arity - 1));
}

static auto isf_gen_transformed_inputs(std::vector<std::string> const& transforms_names, size_t current_arity, size_t desired_arity, std::string const& implicit_conversion) -> std::string
{
    assert(transforms_names.size() == current_arity);

    std::string res{};

    for (size_t i = 0; i < current_arity; ++i)
    {
        if (Cool::String::contains(transforms_names[i], "Voidto") || Cool::String::contains(transforms_names[i], "default_constant"))
            res += fmt::format("{}()", transforms_names[i]);
        else
            res += fmt::format("{}({}({}))", transforms_names[i], implicit_conversion, isf_argument_name(i, desired_arity));
        if (i != current_arity - 1)
            res += ", ";
    }

    return res;
}

static auto isf_gen_implicit_curve_renderer(
    FunctionSignature   desired,
    std::string_view    base_function_name,
    Node const&         node,
    Cool::NodeId const& node_id,
    ISFContext&          context
) -> tl::expected<std::string, std::string>
{
    auto const curve_func_name = isf_gen_desired_function(curve_signature(), node, node_id, context);
    if (!curve_func_name)
        return tl::make_unexpected(curve_func_name.error());
    auto const shape_func_name = fmt::format("curveRenderer{}", valid_glsl(std::string{base_function_name}));
    context.code_gen_context.push_function(Function{
        .name       = "Coollab_sdSegment",
        .definition = R"STR(
// https://iquilezles.org/articles/distfunctions2d/
float Coollab_sdSegment(vec2 p, vec2 a, vec2 b, float thickness)
{{
    vec2  pa = p - a, ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h) - thickness;
}}
        )STR",
    });
    context.code_gen_context.push_function(Function{
        .name       = shape_func_name,
        .definition = fmt::format(R"STR(
float {}/*needs_coollab_context*/(vec2 uv)
{{
    const int NB_SEGMENTS = 300;
    const float THICKNESS = 0.01;

    float dist_to_curve = FLT_MAX;
    vec2  previous_position;

    for (int i = 0; i <= NB_SEGMENTS; i++)
    {{
        float t = i / float(NB_SEGMENTS);

        vec2 current_position = {}(t);
        if (i != 0)
        {{
            float segment = Coollab_sdSegment(uv, previous_position, current_position, THICKNESS);
            dist_to_curve = min(dist_to_curve, segment);
        }}

        previous_position = current_position;
    }}

    return dist_to_curve;
}}
)STR",
                                  shape_func_name, *curve_func_name),
    });
    return isf_gen_desired_function_implementation(shape_2D_signature(), desired, shape_func_name, node, node_id, context);
}

static auto isf_gen_implicit_curve_renderer_3D(
    FunctionSignature   desired,
    std::string_view    base_function_name,
    Node const&         node,
    Cool::NodeId const& node_id,
    ISFContext&          context
) -> tl::expected<std::string, std::string>
{
    auto const curve_func_name = isf_gen_desired_function(curve_3D_signature(), node, node_id, context);
    if (!curve_func_name)
        return tl::make_unexpected(curve_func_name.error());
    auto const shape_func_name = fmt::format("curveRenderer3D{}", valid_glsl(std::string{base_function_name}));
    context.code_gen_context.push_function(Function{
        .name       = "Coollab_sdSegment3D",
        .definition = R"STR(
// https://iquilezles.org/articles/distfunctions/
float Coollab_sdSegment3D(vec3 p, vec3 a, vec3 b, float thickness)
{{
    vec3  pa = p - a, ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h) - thickness;
}}
        )STR",
    });
    context.code_gen_context.push_function(Function{
        .name       = shape_func_name,
        .definition = fmt::format(R"STR(
float {}/*needs_coollab_context*/(vec3 pos)
{{
    const int NB_SEGMENTS = 300;
    const float THICKNESS = 0.01;

    float dist_to_curve = FLT_MAX;
    vec3  previous_position;

    for (int i = 0; i <= NB_SEGMENTS; i++)
    {{
        float t = i / float(NB_SEGMENTS);

        vec3 current_position = {}(t);
        if (i != 0)
        {{
            float segment = Coollab_sdSegment3D(pos, previous_position, current_position, THICKNESS);
            dist_to_curve = min(dist_to_curve, segment);
        }}

        previous_position = current_position;
    }}

    return dist_to_curve;
}}
)STR",
                                  shape_func_name, *curve_func_name),
    });
    return isf_gen_desired_function_implementation(shape_3D_signature(), desired, shape_func_name, node, node_id, context);
}

static auto isf_gen_implicit_shape_3D_renderer(
    FunctionSignature   desired,
    std::string_view    base_function_name,
    Node const&         node,
    Cool::NodeId const& node_id,
    ISFContext&          context
) -> tl::expected<std::string, std::string>
{
    using fmt::literals::operator""_a;

    auto const shape_3D_func_name = isf_gen_desired_function(shape_3D_signature(), node, node_id, context);
    if (!shape_3D_func_name)
        return tl::make_unexpected(shape_3D_func_name.error());
    auto const image_func_name = fmt::format("shape3DRenderer{}", valid_glsl(std::string{base_function_name}));
    context.code_gen_context.push_function(Function{
        .name       = image_func_name,
        .definition = fmt::format(
            FMT_COMPILE(R"STR(
vec4 {image_name}/*needs_coollab_context*/(vec2 uv)
{{
    const int MAX_STEPS = 100;
    const float MAX_DIST = 100.;
    const float SURF_DIST = .001;

    vec3 ro = cool_ray_origin(unnormalize_uv(uv));
    vec3 rd = cool_ray_direction(unnormalize_uv(uv));

    // Ray march
    float d = 0.;
    for (int i = 0; i < MAX_STEPS; i++)
    {{
        vec3  p  = ro + rd * d;
        float dS = {shape_3D}(p);
        d += dS;
        if (d > MAX_DIST || abs(dS) < SURF_DIST)
            break;
    }}

    // Background
    if (d >= MAX_DIST)
        return vec4(0.);

    // Return the normal as a color
    vec3 p = ro + rd * d;
    const vec2 e = vec2(.001, 0);
    vec3 normal = {shape_3D}(p)-vec3({shape_3D}(p - e.xyy), {shape_3D}(p - e.yxy), {shape_3D}(p - e.yyx));
    normal = normalize(normal) * 0.5 + 0.5;
    return vec4(normal, 1.);
}}
)STR"),
            "image_name"_a = image_func_name,
            "shape_3D"_a   = *shape_3D_func_name
        ),
    });
    auto const image_signature = FunctionSignature{.from = PrimitiveType::UV, .to = PrimitiveType::LinearRGB_StraightA, .arity = 1};
    return isf_gen_desired_function_implementation(image_signature, desired, image_func_name, node, node_id, context);
}

static auto isf_gen_desired_function_implementation(
    FunctionSignature   current,
    FunctionSignature   desired,
    std::string_view    base_function_name,
    Node const&         node,
    Cool::NodeId const& node_id,
    ISFContext&          context
) -> tl::expected<std::string, std::string>
{
    using fmt::literals::operator""_a;

    if (is_curve(current) && !is_curve(desired))
        return isf_gen_implicit_curve_renderer(desired, base_function_name, node, node_id, context);
    if (is_curve_3D(current) && !is_curve_3D(desired))
        return isf_gen_implicit_curve_renderer_3D(desired, base_function_name, node, node_id, context);
    if (is_shape_3D(current) && !is_shape_3D(desired))
        return isf_gen_implicit_shape_3D_renderer(desired, base_function_name, node, node_id, context);

    auto const implicit_conversions = gen_implicit_conversions(current, desired, context.code_gen_context);

    auto input_transformation_names = std::vector<std::string>{};
    input_transformation_names.reserve(current.arity);
    for (size_t i = 0; i < current.arity; ++i)
    {
        auto const input_transformation_name = isf_input_transformation(current, desired, implicit_conversions)
                                                   .gen_func(node.number_of_main_input_pins() > i ? node.main_input_pin(i) : Cool::InputPin{}, context);
        if (!input_transformation_name)
            return input_transformation_name;

        input_transformation_names.push_back(*input_transformation_name);
    }

    auto const output_transformation_name = isf_output_transformation(current, desired, implicit_conversions)
                                                .gen_func(node.number_of_main_input_pins() > 0 ? node.main_input_pin(0) : Cool::InputPin{}, context);
    if (!output_transformation_name)
        return output_transformation_name;

    auto const call_base_function = fmt::format(
        FMT_COMPILE("{base_function}({inputs})"),
        "base_function"_a = base_function_name,
        "inputs"_a        = isf_gen_transformed_inputs(input_transformation_names, current.arity, desired.arity, implicit_conversions.input.value_or(""))
    );

    auto const does_output_uv = current.to == PrimitiveType::UV
                                || (current.to == PrimitiveType::Vec2 && desired.to == PrimitiveType::UV);
    return fmt::format(
        FMT_COMPILE(R"STR(
{store_uv}
return {transform_output}({implicit_output_conversion}({base_function_output}));
)STR"),
        "store_uv"_a                   = does_output_uv ? fmt::format("coollab_context.uv = {};", call_base_function) : "",
        "base_function_output"_a       = does_output_uv ? "coollab_context.uv" : call_base_function,
        "transform_output"_a           = *output_transformation_name,
        "implicit_output_conversion"_a = implicit_conversions.output.value_or("")
    );
}

// ---- InputPin overload of isf_gen_desired_function ----

static auto isf_gen_output_function(Cool::OutputPin const& pin, ISFContext& context)
    -> ExpectedFunctionName
{
    auto const output_name = make_valid_output_index_name(pin);
    auto const func_name   = fmt::format("get{}", output_name);

    return context.code_gen_context.push_function({
        .name       = func_name,
        .definition = fmt::format(
            R"STR(
vec2 {}()
{{
    return vec2({}, 1.); // Convert float to float_and_alpha
}}
)STR",
            func_name, output_name
        ),
    });
}

static auto isf_gen_desired_function(
    FunctionSignature     desired_signature,
    Cool::InputPin const& pin,
    ISFContext&            context,
    bool                  fallback_to_a_default_function
) -> ExpectedFunctionName
{
    Cool::OutputPin output_pin;
    auto const      node_id = context.graph().find_node_connected_to_input_pin(pin.id(), &output_pin);
    auto const      node    = context.graph().try_get_node<Node>(node_id);

    if (node && output_pin != node->main_output_pin())
    {
        return isf_gen_output_function(output_pin, context);
    }

    if (!node && !fallback_to_a_default_function)
        return "";

    return isf_gen_desired_function(
        desired_signature,
        node,
        node_id,
        context
    );
}

// ---- Main traversal function ----

static auto isf_gen_desired_function(
    FunctionSignature   desired_signature,
    Node const*         maybe_node,
    Cool::NodeId const& id,
    ISFContext&          context
) -> ExpectedFunctionName
{
    if (!maybe_node)
    {
        // Generate a default function (black/transparent)
        // Reuse the existing default function gen with a dummy MaybeGenerateModule
        MaybeGenerateModule dummy = [](Cool::NodeId const&, NodeDefinition const&) -> MaybeTextureName {
            return None{};
        };
        return gen_default_function(desired_signature, context.code_gen_context);
    }

    return isf_gen_desired_function(desired_signature, *maybe_node, id, context);
}

static auto isf_gen_desired_function(
    FunctionSignature                  desired_signature,
    std::reference_wrapper<Node const> node_ref,
    Cool::NodeId const&                id,
    ISFContext&                         context
) -> ExpectedFunctionName
{
    auto const& node            = node_ref.get();
    auto const* node_definition = context.get_node_definition(node.id_names());
    if (!node_definition)
        return tl::make_unexpected(fmt::format("Node definition '{}' was not found.", node.definition_name()));

    auto const& def_name = node_definition->name();

    // ---- Check for unsupported nodes ----
    if (is_particle(node_definition->signature()))
    {
        context.errors.push_back(fmt::format("Particle nodes ('{}') are not supported in ISF export.", def_name));
        return gen_default_function(desired_signature, context.code_gen_context);
    }
    if (is_jfa(*node_definition))
    {
        context.errors.push_back("Mask to Shape (JFA) nodes are not supported in ISF export.");
        return gen_default_function(desired_signature, context.code_gen_context);
    }
    if (is_feedback_loop(*node_definition))
    {
        context.errors.push_back("Feedback (One frame delay) nodes are not supported in ISF export yet.");
        return gen_default_function(desired_signature, context.code_gen_context);
    }
    if (is_select_node(def_name) && select_needs_isf_input(node, *node_definition, context.graph()))
    {
        // Selector pin is unconnected (or MIDI variant) — create an ISF long INPUT dropdown.
        // When the selector IS connected (e.g. driven by a Time or Input node), we fall through
        // to the generic code gen path which will properly traverse the connection.
        return handle_select_node(node, *node_definition, id, desired_signature, context);
    }

    // ---- Handle special nodes ----
    ExpectedFunctionName base_func_name;

    if (def_name == "Input Number" || def_name == "Input Integer" || def_name == "Input Angle"
        || def_name == "Input Point 2D" || def_name == "Input Color" || def_name == "Input Vec2"
        || def_name == "Input Bool" || def_name == "Input Boolean")
    {
        base_func_name = handle_input_node(node, *node_definition, id, context);
    }
    else if (def_name == "Time")
    {
        base_func_name = handle_time_node(node, *node_definition, id, context);
    }
    else if (def_name == "Volume (Audio)")
    {
        base_func_name = handle_volume_audio_node(*node_definition, id, context);
    }
    else if (def_name == "Spectrum (Audio)")
    {
        // Audio handlers generate the desired signature directly to avoid
        // type mismatch bugs in the generic conversion machinery (see `ISF Export v1 to v2.md`).
        return handle_spectrum_audio_node(node, *node_definition, id, desired_signature, context);
    }
    else if (def_name == "Waveform (Audio)")
    {
        return handle_waveform_audio_node(node, *node_definition, id, desired_signature, context);
    }
    else if (def_name == "Image")
    {
        base_func_name = handle_image_node(node, *node_definition, id, context);
    }
    else if (def_name == "Webcam")
    {
        base_func_name = handle_webcam_node(node, *node_definition, id, context);
    }
    else if (def_name == "Midi")
    {
        base_func_name = handle_midi_node(node, *node_definition, id, context);
    }
    else if (def_name == "OSC")
    {
        base_func_name = handle_osc_node(node, *node_definition, id, context);
    }
    else if (def_name == "Video")
    {
        context.errors.push_back("Video nodes are not supported in ISF export.");
        return gen_default_function(desired_signature, context.code_gen_context);
    }
    else if (is_caching(*node_definition))
    {
        base_func_name = handle_caching_node(node, *node_definition, id, context);
    }
    else
    {
        // Generic node: generate normally with constants instead of uniforms
        base_func_name = isf_gen_base_function(node, *node_definition, id, context);
    }

    if (!base_func_name)
        return tl::make_unexpected(fmt::format("Failed to generate code for node '{}':\n{}", def_name, base_func_name.error()));

    // If the base function already has the desired signature, return it directly
    if (node_definition->signature() == desired_signature)
        return *base_func_name;

    // Use our ISF version of gen_desired_function_implementation which redirects
    // all recursive gen_desired_function calls through the ISF path, so nodes
    // reached through type conversions produce ISF inputs/constants instead of uniforms.
    auto const func_body = isf_gen_desired_function_implementation(
        node_definition->signature(),
        desired_signature,
        *base_func_name,
        node,
        id,
        context);
    if (!func_body)
        return tl::make_unexpected(fmt::format("Failed to generate conversion for node '{}':\n{}", def_name, func_body.error()));

    auto const func_name = desired_function_name(*node_definition, id, desired_signature);

    auto make_arguments_list = [](size_t arity, PrimitiveType type) -> std::string {
        std::string res;
        for (size_t i = 0; i < arity; ++i)
        {
            res += fmt::format("{} in{}", glsl_type_as_string(type), i);
            if (i != arity - 1)
                res += ", ";
        }
        return res;
    };

    auto const arguments_list = make_arguments_list(desired_signature.arity, desired_signature.from);
    auto const func_def       = gen_function_definition({
              .signature_as_string = {glsl_type_as_string(desired_signature.to), "", arguments_list},
              .unique_name         = func_name,
              .before_function     = "",
              .body                = *func_body,
    });

    context.code_gen_context.push_function({.name = func_name, .definition = func_def});
    return func_name;
}

// ---- Public API ----

auto isf_generate_code(
    Cool::NodeId const&                         root_node_id,
    FunctionSignature const&                    signature,
    Cool::NodesGraph const&                     graph,
    Cool::GetNodeDefinition_Ref<NodeDefinition> get_node_definition
) -> ISFCodeGenResult
{
    auto context = ISFContext{
        .code_gen_context = CodeGenContext{graph, get_node_definition},
    };

    auto const main_function_name = isf_gen_desired_function(
        signature,
        graph.try_get_node<Node>(root_node_id),
        root_node_id,
        context);

    ISFCodeGenResult result;
    result.metadata      = std::move(context.metadata);
    result.errors        = std::move(context.errors);
    result.warnings      = std::move(context.warnings);
    result.files_to_copy = std::move(context.files_to_copy);
    result.cache_passes  = std::move(context.cache_passes);

    if (!main_function_name)
    {
        result.errors.push_back(fmt::format("Failed to generate shader code: {}", main_function_name.error()));
        return result;
    }

    // Inject CoollabContext argument into all functions (same post-processing as the normal pipeline)
    // This is done by generate_shader_code.cpp normally
    auto code = context.code_gen_context.code();

    // Inject CoollabContext argument — same logic as inject_context_argument_in_all_functions() in generate_shader_code.cpp
    {
        using namespace std::string_view_literals;
        static constexpr auto magic_comment  = "/*needs_coollab_context*/("sv;
        static constexpr auto ctx_declaration = "CoollabContext coollab_context, "sv;

        auto func_names = std::set<std::string>{};
        auto pos        = code.find(magic_comment);
        while (pos != std::string_view::npos)
        {
            auto const func_name = Cool::String::find_previous_word(code, pos);
            if (func_name)
                func_names.insert(*func_name);

            code.insert(pos + magic_comment.size(), ctx_declaration);
            pos += magic_comment.size() + ctx_declaration.size();
            pos = code.find(magic_comment, pos);
        }

        for (auto const& fn : func_names)
            Cool::String::replace_all_beginnings_of_words_inplace(code, fn + "(", fn + "(coollab_context, ");

        Cool::String::replace_all_inplace(code, ", )", ")");
        Cool::String::replace_all_inplace(code, "(coollab_context, ()", "(coollab_context");
    }

    // Remove extra parentheses
    Cool::String::replace_all_inplace(code, "(())", "()");

    result.glsl_code          = std::move(code);
    result.main_function_name = *main_function_name;

    return result;
}

} // namespace Lab
