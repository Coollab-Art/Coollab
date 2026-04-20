#include "ISFExport.h"
#include <Cool/Dependencies/AnySharedVariable.h>
#include <Cool/File/File.h>
#include <Cool/StrongTypes/ColorAndAlpha.h>
#include <Cool/StrongTypes/ColorPalette.h>
#include <Cool/StrongTypes/Gradient.h>
#include <Cool/Gpu/OpenGL/preprocess_shader_source.h>
#include <Cool/Path/Path.h>
#include <Cool/String/String.h>
#include <Cool/Utils/overloaded.hpp>
#include <Cool/Variables/gen_input_shader_code.h>
#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include "ISFCodeGen.h"
#include "ISFMetadata.h"
#include "Nodes/CodeGen.h"
#include "Nodes/FunctionSignature.h"
#include "Nodes/Node.h"
#include "Nodes/valid_glsl.h"
#include "Nodes/valid_input_name.h"

namespace Lab {

/// Removes non-ASCII characters from code.
/// The v2 ISF compiler's replace_keyword uses `bytes[i] as char` which corrupts
/// non-ASCII bytes, causing exponential string growth and stack overflow.
static void strip_non_ascii(std::string& code)
{
    std::string result;
    result.reserve(code.size());
    for (char c : code)
    {
        if (static_cast<unsigned char>(c) <= 127)
            result += c;
    }
    code = std::move(result);
}

/// Resolves preprocessor conditionals for the ISF export context.
/// We are NOT a compute shader, MIXBOX_INCLUDED will be defined, PI/TWO_PI are defined.
/// This strips all #ifndef/#ifdef/#endif/#if blocks by evaluating them statically.
static void resolve_preprocessor(std::string& code)
{
    std::istringstream stream(code);
    std::string        result;
    std::string        line;
    // Stack of booleans: true = currently emitting lines, false = skipping
    std::vector<bool> emit_stack;
    emit_stack.push_back(true); // Top-level: always emit

    while (std::getline(stream, line))
    {
        auto const trimmed_pos = line.find_first_not_of(" \t");
        auto const trimmed     = (trimmed_pos != std::string::npos) ? line.substr(trimmed_pos) : "";

        if (trimmed.rfind("#ifndef ", 0) == 0)
        {
            auto const macro = trimmed.substr(8);
            // COOL_COMPUTE_SHADER is not defined (we're a fragment shader)
            // MIXBOX_INCLUDED will be defined (we include mixbox)
            // PI, TWO_PI are defined (from math.glsl)
            // MIXBOX_LUT is defined
            bool defined = (macro.find("COOL_COMPUTE_SHADER") != std::string::npos) ? false
                           : (macro.find("MIXBOX_INCLUDED") != std::string::npos)    ? true
                           : (macro.find("MIXBOX_LUT") != std::string::npos)         ? true
                                                                                     : false;
            emit_stack.push_back(!defined); // ifndef: emit if NOT defined
        }
        else if (trimmed.rfind("#ifdef ", 0) == 0)
        {
            auto const macro = trimmed.substr(7);
            bool       defined = (macro.find("MIXBOX_COLORSPACE_LINEAR") != std::string::npos) ? false
                                 : (macro.find("COOL_COMPUTE_SHADER") != std::string::npos)    ? false
                                                                                               : false;
            emit_stack.push_back(defined); // ifdef: emit if defined
        }
        else if (trimmed.rfind("#if ", 0) == 0)
        {
            emit_stack.push_back(true); // Conservatively emit #if blocks
        }
        else if (trimmed == "#else")
        {
            if (emit_stack.size() > 1)
                emit_stack.back() = !emit_stack.back();
        }
        else if (trimmed.rfind("#elif ", 0) == 0)
        {
            // Simplified: treat like #else (invert current condition).
            // Not fully correct for arbitrary expressions, but sufficient for
            // the limited set of macros we handle in our helper GLSL.
            if (emit_stack.size() > 1)
                emit_stack.back() = !emit_stack.back();
        }
        else if (trimmed.rfind("#endif", 0) == 0)
        {
            if (emit_stack.size() > 1)
                emit_stack.pop_back();
        }
        else
        {
            // Emit line if all levels of the stack say yes
            bool emit = true;
            for (bool b : emit_stack)
            {
                if (!b)
                {
                    emit = false;
                    break;
                }
            }
            if (emit)
                result += line + '\n';
        }
    }
    code = std::move(result);
}

/// Removes all #include lines (helper GLSL is already inlined).
static void strip_include_lines(std::string& code)
{
    std::istringstream stream(code);
    std::string        result;
    std::string        line;
    while (std::getline(stream, line))
    {
        auto const trimmed_pos = line.find_first_not_of(" \t");
        if (trimmed_pos != std::string::npos && line.substr(trimmed_pos, 8) == "#include")
            continue;
        result += line + '\n';
    }
    code = std::move(result);
}

/// Removes all lines that start with "uniform " (after optional whitespace).
static void strip_uniform_lines(std::string& code)
{
    std::istringstream stream(code);
    std::string        result;
    std::string        line;
    while (std::getline(stream, line))
    {
        // Find first non-whitespace
        auto const first_non_space = line.find_first_not_of(" \t");
        if (first_non_space != std::string::npos && line.substr(first_non_space, 8) == "uniform ")
            continue; // Skip this line entirely
        result += line + '\n';
    }
    code = std::move(result);
}

/// Collects constant value strings for all value inputs across all nodes in the graph.
/// Returns a map from valid_input_name to "type name = value;" string.
static auto collect_constant_values(Cool::NodesGraph const& graph) -> std::unordered_map<std::string, std::string>
{
    std::unordered_map<std::string, std::string> constants;
    graph.for_each_node<Node>([&](Node const& node) {
        for (auto const& prop : node.value_inputs())
        {
            auto const name  = valid_input_name(prop);
            auto const value = std::visit(
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
                    [&](Cool::SharedVariable<Cool::TimeSpeed> const& v) -> std::string {
                        return fmt::format("float {} = {};", name, v.value().value);
                    },
                    [&](Cool::SharedVariable<Cool::Time> const& v) -> std::string {
                        return fmt::format("float {} = {};", name, v.value().as_seconds_double());
                    },
                    [&](Cool::SharedVariable<Cool::Gradient> const& v) -> std::string {
                        // Generate a const array of GradientMark with baked values.
                        // The array name in generated code is: valid_input_name(prop) + "_"
                        // (because gradient_marks_array_name appends "_" to the name, and name replacement
                        //  turns 'GradientName' into valid_input_name)
                        auto const& marks      = v.value().value.gradient().get_marks();
                        auto const  array_name = name + "_"; // valid_input_name already computed above
                        if (marks.empty())
                            return "";
                        std::string array_init = fmt::format("const GradientMark {}[{}] = GradientMark[{}](\n", array_name, marks.size(), marks.size());
                        size_t j = 0;
                        for (auto const& mark : marks)
                        {
                            auto const col = Cool::ColorAndAlpha::from_srgb_straight_alpha(glm::vec4(mark.color.x, mark.color.y, mark.color.z, mark.color.w)).as_Oklab_PremultipliedA();
                            array_init += fmt::format("    GradientMark({}, vec4({}, {}, {}, {}))", mark.position.get(), col.x, col.y, col.z, col.w);
                            if (j + 1 < marks.size())
                                array_init += ",";
                            array_init += "\n";
                            j++;
                        }
                        array_init += ");\n";
                        constants[array_name] = array_init;
                        return "";
                    },
                    [&](Cool::SharedVariable<Cool::ColorPalette> const& v) -> std::string {
                        // Same pattern: array name is valid_input_name(prop) + "_"
                        auto const  array_name = name + "_";
                        auto const& colors     = v.value().value;
                        if (colors.empty())
                            return "";
                        std::string array_init = fmt::format("const vec3 {}[{}] = vec3[{}](\n", array_name, colors.size(), colors.size());
                        for (size_t j = 0; j < colors.size(); ++j)
                        {
                            auto const srgb = colors[j].as_sRGB();
                            array_init += fmt::format("    vec3({}, {}, {})", srgb.x, srgb.y, srgb.z);
                            if (j + 1 < colors.size())
                                array_init += ",";
                            array_init += "\n";
                        }
                        array_init += ");\n";
                        constants[array_name] = array_init;
                        return "";
                    },
                    [&](auto const&) -> std::string {
                        return ""; // Other types (Texture, etc.) are handled differently
                    },
                },
                prop);
            if (!value.empty())
                constants[name] = value;
        }
    });
    return constants;
}

/// Replaces "uniform type name;" declarations in generated code with "type name = VALUE;"
/// using actual values from the node graph.
static void initialize_uniforms_with_constants(std::string& code, Cool::NodesGraph const& graph)
{
    auto const constants = collect_constant_values(graph);

    std::istringstream stream(code);
    std::string        result;
    std::string        line;
    while (std::getline(stream, line))
    {
        auto const trimmed_pos = line.find_first_not_of(" \t");
        if (trimmed_pos != std::string::npos && line.substr(trimmed_pos, 8) == "uniform ")
        {
            // Extract the variable name from "uniform type name;" or "uniform type name[N];"
            auto const without_uniform = line.substr(trimmed_pos + 8);
            // Find the name: last word before ';' or '['
            auto end = without_uniform.find(';');
            auto bracket = without_uniform.find('[');
            if (bracket != std::string::npos && (end == std::string::npos || bracket < end))
                end = bracket;
            if (end != std::string::npos)
            {
                auto name_end = end;
                while (name_end > 0 && std::isspace(without_uniform[name_end - 1]))
                    name_end--;
                auto name_start = name_end;
                while (name_start > 0 && (std::isalnum(without_uniform[name_start - 1]) || without_uniform[name_start - 1] == '_'))
                    name_start--;
                auto const var_name = without_uniform.substr(name_start, name_end - name_start);

                auto const it = constants.find(var_name);
                if (it != constants.end())
                {
                    result += it->second + '\n';
                    continue;
                }
            }
            // If not found in constants: strip types that ISF can't handle
            auto const rest = line.substr(trimmed_pos + 8);
            if (rest.find("sampler") == 0)
            {
                result += "// (stripped unsupported sampler uniform for ISF)\n";
            }
            else if (rest.find("Cool_Texture") == 0)
            {
                // Keep as a plain global using the dummy Cool_Texture struct (no sampler2D).
                // Unconnected texture inputs will produce transparent black.
                result += rest + '\n';
            }
            else
            {
                result += rest + '\n';
            }
        }
        else
        {
            result += line + '\n';
        }
    }
    code = std::move(result);
}

static auto read_and_preprocess_helper_glsl() -> tl::expected<std::string, std::string>
{
    // Build a GLSL source with all the includes we need, then preprocess it to resolve them.
    // We exclude Texture.glsl because it defines Cool_Texture with sampler2D which ISF can't handle.
    auto const source = std::string{R"glsl(
#include "_COOL_RES_/shaders/shader-utils.glsl"
#include "_ROOT_FOLDER_/res/mixbox/mixbox.glsl"
#include "_COOL_RES_/shaders/math.glsl"
#include "_COOL_RES_/shaders/rand.glsl"
#include "_COOL_RES_/shaders/color_conversions.glsl"
#include "_COOL_RES_/shaders/camera.glsl"
#include "_COOL_RES_/shaders/GradientMark.glsl"
)glsl"};

    auto result = Cool::OpenGL::preprocess_shader_source(source);
    if (!result)
        return tl::make_unexpected(fmt::format("Failed to preprocess helper GLSL:\n{}", result.error()));

    auto& code = *result;

    // Resolve preprocessor conditionals (#ifndef COOL_COMPUTE_SHADER, etc.)
    resolve_preprocessor(code);

    // Replace layout declarations and uniforms that need to become ISF-compatible globals
    Cool::String::replace_all_inplace(code, "layout(location = 0) in vec2 _uv;", "vec2 _uv; // set in main()");
    Cool::String::replace_all_inplace(code, "uniform float _aspect_ratio;", "float _aspect_ratio; // set in main()");

    // Replace 3D camera uniforms with baked constants — replaced with actual values in assemble step
    Cool::String::replace_all_inplace(code, "uniform mat4  cool_camera_view_projection;", "mat4 cool_camera_view_projection; // baked in main()");
    Cool::String::replace_all_inplace(code, "uniform mat4  cool_camera_inverse_view_projection;", "mat4 cool_camera_inverse_view_projection; // baked in main()");
    Cool::String::replace_all_inplace(code, "uniform float cool_camera_far_plane;", "float cool_camera_far_plane; // baked in main()");

    // Strip all remaining uniform declarations (ISF provides its own via inputs/imported)
    strip_uniform_lines(code);

    return code;
}

static auto format_mat3_constant(std::string const& name, glm::mat3 const& m) -> std::string
{
    return fmt::format(
        "const mat3 {} = mat3({}, {}, {}, {}, {}, {}, {}, {}, {});\n",
        name,
        m[0][0], m[0][1], m[0][2],
        m[1][0], m[1][1], m[1][2],
        m[2][0], m[2][1], m[2][2]
    );
}

static auto format_mat4_assignment(std::string const& name, glm::mat4 const& m) -> std::string
{
    return fmt::format(
        "    {} = mat4({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});\n",
        name,
        m[0][0], m[0][1], m[0][2], m[0][3],
        m[1][0], m[1][1], m[1][2], m[1][3],
        m[2][0], m[2][1], m[2][2], m[2][3],
        m[3][0], m[3][1], m[3][2], m[3][3]
    );
}

static auto replace_global_uniforms(std::string& code, glm::mat3 const& cam_transform, glm::mat3 const& cam_view) -> std::string
{
    // Collect any definitions we need to prepend
    std::string prepended;

    // Replace time-related uniforms
    Cool::String::replace_all_words_inplace(code, "_time", "TIME");
    Cool::String::replace_all_words_inplace(code, "_delta_time", "TIMEDELTA");
    Cool::String::replace_all_words_inplace(code, "_height", "RENDERSIZE.y");

    // Replace camera uniforms with baked constants
    Cool::String::replace_all_words_inplace(code, "_camera2D_transform", "isf_baked_camera2D_transform");
    Cool::String::replace_all_words_inplace(code, "_camera2D_view", "isf_baked_camera2D_view");
    prepended += format_mat3_constant("isf_baked_camera2D_transform", cam_transform);
    prepended += format_mat3_constant("isf_baked_camera2D_view", cam_view);

    // Replace MIDI button uniforms with constants (not available in ISF)
    Cool::String::replace_all_words_inplace(code, "_last_midi_button_pressed", "0");
    Cool::String::replace_all_words_inplace(code, "_last_last_midi_button_pressed", "0");
    Cool::String::replace_all_words_inplace(code, "_time_since_last_midi_button_pressed", "0.0");

    // The _audio_volume, _audio_spectrum, _audio_waveform uniforms are handled
    // by the special audio node handlers. If any remain, replace with defaults.
    Cool::String::replace_all_words_inplace(code, "_audio_volume", "0.0");

    return prepended;
}

static auto gen_output_indices_declarations(Cool::NodesGraph const& graph) -> std::string
{
    std::string res;
    graph.for_each_node<Node>([&](Node const& node) {
        for (size_t i = 1; i < node.output_pins().size(); ++i)
            res += fmt::format("float {};\n", make_valid_output_index_name(node.output_pins()[i]));
    });
    return res;
}

auto export_as_isf(
    ISFExportParams const&       params,
    std::filesystem::path const& output_path
) -> tl::expected<ISFExportResult, std::string>
{
    // Step 1: Generate code via graph traversal
    auto const signature = FunctionSignature{
        .from  = PrimitiveType::UV,
        .to    = PrimitiveType::sRGB_StraightA,
        .arity = 1,
    };

    auto result = isf_generate_code(
        params.root_node_id,
        signature,
        params.graph,
        params.get_node_definition
    );

    // Step 2: Check for errors (warnings are collected but don't abort)
    if (!result.errors.empty())
    {
        std::string error_msg = "ISF export encountered unsupported features:\n";
        for (auto const& err : result.errors)
            error_msg += fmt::format("  - {}\n", err);
        return tl::make_unexpected(error_msg);
    }
    auto warnings = std::move(result.warnings);

    if (result.main_function_name.empty())
        return tl::make_unexpected("Failed to generate shader code: no main function produced.");

    // If there are caching passes, add the final screen pass (empty target = render to screen)
    if (!result.cache_passes.empty())
        result.metadata.passes.push_back({"", false});

    // Step 3: Read and preprocess helper GLSL
    auto helper_glsl = read_and_preprocess_helper_glsl();
    if (!helper_glsl)
        return tl::make_unexpected(helper_glsl.error());

    // Step 4: Replace global uniforms in the generated code
    auto glsl_code = result.glsl_code;

    // Strip #include directives from generated code (helper GLSL is already inlined above)
    strip_include_lines(glsl_code);

    auto prepended = replace_global_uniforms(glsl_code, params.camera_2D_transform, params.camera_2D_view);

    // Replace "uniform type name;" with "type name = VALUE;" using actual values from the node graph.
    // Some nodes' value inputs get generated as uniforms by the original code gen path
    // (called from gen_desired_function_implementation for type conversions).
    // We need to initialize them with the correct constant values.
    initialize_uniforms_with_constants(glsl_code, params.graph);

    // Also replace in helper GLSL (some helpers reference _time etc.)
    replace_global_uniforms(*helper_glsl, params.camera_2D_transform, params.camera_2D_view);

    // Convert texture(name, uv) calls to IMG_NORM_PIXEL(name, uv) for ISF imported/input images.
    // The MaybeGenerateModule callback creates fake texture-read nodes that produce
    // texture(imported_image_X, uv) — we convert those to ISF syntax.
    // Also handle texture(X.tex, uv) from unconnected Cool_Texture inputs → vec4(0.).
    for (auto const& [isf_name, _] : result.metadata.imported)
        Cool::String::replace_all_inplace(glsl_code, fmt::format("texture({},", isf_name), fmt::format("IMG_NORM_PIXEL({},", isf_name));
    for (auto const& input : result.metadata.inputs)
    {
        if (std::holds_alternative<ISFInputImage>(input.data))
            Cool::String::replace_all_inplace(glsl_code, fmt::format("texture({},", input.name), fmt::format("IMG_NORM_PIXEL({},", input.name));
    }
    // Replace remaining texture(X.tex, ...) from unconnected Cool_Texture inputs with vec4(0.)
    Cool::String::replace_all_inplace(glsl_code, ".tex,", ".aspect_ratio,"); // hack: make .tex references compile by redirecting to aspect_ratio (texture() call will still fail)
    {
        size_t pos = 0;
        while ((pos = glsl_code.find("texture(", pos)) != std::string::npos)
        {
            // Check if this is a Cool_Texture .aspect_ratio hack (redirected from .tex)
            auto const inner_start = pos + 8;
            auto const dot_pos     = glsl_code.find(".aspect_ratio,", inner_start);
            if (dot_pos != std::string::npos && dot_pos < inner_start + 80)
            {
                // Find closing paren
                int    depth = 1;
                size_t p     = inner_start;
                while (p < glsl_code.size() && depth > 0)
                {
                    if (glsl_code[p] == '(') depth++;
                    else if (glsl_code[p] == ')') depth--;
                    p++;
                }
                glsl_code.replace(pos, p - pos, "vec4(0.)");
            }
            pos += 1;
        }
    }

    // Step 5: Check if mixbox LUT is needed
    bool uses_mixbox = glsl_code.find("mixbox_lut") != std::string::npos
                       || helper_glsl->find("mixbox_lut") != std::string::npos;
    if (uses_mixbox)
    {
        // Add mixbox LUT as an IMPORTED texture
        auto const lut_filename                = "mixbox_lut.png";
        result.metadata.imported["mixbox_lut"] = lut_filename;

        // Copy the LUT file
        auto const lut_source = Cool::Path::root() / "res" / "mixbox" / "mixbox_lut.png";
        result.files_to_copy.push_back({lut_source, lut_filename});
    }

    // Step 6: Assemble the final ISF file
    auto const output_indices = gen_output_indices_declarations(params.graph);

    // Bake the 3D camera view matrix and projection parameters; compute view_projection dynamically from aspect ratio
    auto const& v = params.camera_3D_view;
    auto cam3d_assignments = fmt::format(
        R"glsl(    // 3D camera: compute projection from current aspect ratio
    float isf_aspect = RENDERSIZE.x / RENDERSIZE.y;
    mat4 isf_cam3d_view = mat4({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
    float isf_fov = {};
    float isf_near = {};
    float isf_far = {};
    float isf_f = 1.0 / tan(isf_fov / 2.0);
    mat4 isf_cam3d_proj = mat4(
        isf_f / isf_aspect, 0.0, 0.0, 0.0,
        0.0, isf_f, 0.0, 0.0,
        0.0, 0.0, (isf_far + isf_near) / (isf_near - isf_far), -1.0,
        0.0, 0.0, (2.0 * isf_far * isf_near) / (isf_near - isf_far), 0.0
    );
    cool_camera_view_projection = isf_cam3d_proj * isf_cam3d_view;
    cool_camera_inverse_view_projection = inverse(cool_camera_view_projection);
    cool_camera_far_plane = isf_far;
)glsl",
        v[0][0], v[0][1], v[0][2], v[0][3],
        v[1][0], v[1][1], v[1][2], v[1][3],
        v[2][0], v[2][1], v[2][2], v[2][3],
        v[3][0], v[3][1], v[3][2], v[3][3],
        params.camera_3D_fov,
        params.camera_3D_near_plane,
        params.camera_3D_far_plane
    );

    // Build the PASSINDEX dispatch (if there are cache passes) or a simple call
    std::string pass_dispatch;
    if (!result.cache_passes.empty())
    {
        for (size_t i = 0; i < result.cache_passes.size(); ++i)
        {
            pass_dispatch += fmt::format(
                "    {} (PASSINDEX == {}) gl_FragColor = {}(coollab_context, uv);\n",
                i == 0 ? "if" : "else if",
                i,
                result.cache_passes[i].function_name);
        }
        pass_dispatch += fmt::format("    else gl_FragColor = {}(coollab_context, uv);\n", result.main_function_name);
    }
    else
    {
        pass_dispatch = fmt::format("    gl_FragColor = {}(coollab_context, uv);\n", result.main_function_name);
    }

    auto const main_function = fmt::format(R"glsl(
void main()
{{
    _uv = isf_FragNormCoord;
    _aspect_ratio = RENDERSIZE.x / RENDERSIZE.y;
{}    vec2 uv = normalize_uv(_uv);
    vec3 tmp = isf_baked_camera2D_transform * vec3(uv, 1.);
    uv = tmp.xy / tmp.z;
    CoollabContext coollab_context;
    coollab_context.uv = uv;
{}}}
)glsl",
                                           cam3d_assignments, pass_dispatch);

    std::string final_code;
    final_code += result.metadata.to_json();
    final_code += "\n";
    final_code += "// --- Helper GLSL (inlined from Coollab v1) ---\n\n";
    final_code += *helper_glsl;
    final_code += "\n";
    final_code += prepended;
    final_code += "\n";
    final_code += "struct CoollabContext { vec2 uv; };\n\n";
    // Dummy Cool_Texture for unconnected texture inputs (no sampler2D in ISF).
    // Provides .aspect_ratio and .flip_y so code referencing them still compiles.
    final_code += R"glsl(struct Cool_Texture {
    float aspect_ratio;
    bool  flip_y;
};
vec4 sample_cool_texture(Cool_Texture tex, vec2 uv) { return vec4(0.); }

)glsl";
    final_code += R"glsl(vec2 to_view_space(vec2 uv)
{
    vec3 p = isf_baked_camera2D_view * vec3(uv, 1.);
    return p.xy / p.z;
}

)glsl";
    final_code += output_indices;
    final_code += "\n";
    final_code += "// --- Generated node functions ---\n\n";
    final_code += glsl_code;
    final_code += "\n";
    final_code += "// --- Main ---\n";
    final_code += main_function;

    // Step 7: Write the .fs file
    {
        auto const output_dir = output_path.parent_path();
        if (!output_dir.empty())
            std::filesystem::create_directories(output_dir);

        std::ofstream file(output_path);
        if (!file.is_open())
            return tl::make_unexpected(fmt::format("Failed to open output file: {}", output_path.string()));

        file << final_code;
        file.close();
    }

    // Step 8: Copy image files
    for (auto const& [source_path, dest_filename] : result.files_to_copy)
    {
        auto const dest_path = output_path.parent_path() / dest_filename;
        try
        {
            if (std::filesystem::exists(source_path))
                std::filesystem::copy_file(source_path, dest_path, std::filesystem::copy_options::overwrite_existing);
        }
        catch (std::exception const& e)
        {
            warnings.push_back(fmt::format("Failed to copy '{}': {}", source_path.string(), e.what()));
        }
    }

    return ISFExportResult{std::move(warnings)};
}

} // namespace Lab
