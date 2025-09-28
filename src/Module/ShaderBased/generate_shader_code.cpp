#include "generate_shader_code.hpp"
#include "Cool/String/String.h"
#include "Nodes/CodeGen.h"
#include "Nodes/Node.h"

namespace Lab {

static auto gen_all_output_indices_declarations(Cool::NodesGraph const& graph)
    -> std::string
{
    std::stringstream res{};
    graph.for_each_node<Node>([&](Node const& node) {
        for (size_t i = 1; i < node.output_pins().size(); ++i)
            res << fmt::format("float {};\n", make_valid_output_index_name(node.output_pins()[i]));
    });
    return res.str();
}

static void inject_context_argument_in_all_functions(std::string& code)
{
    static constexpr auto magic_comment = "/*needs_coollab_context*/("sv;

    auto func_names = std::set<std::string>{};
    auto pos        = code.find(magic_comment);
    while (pos != std::string_view::npos)
    {
        auto const func_name = Cool::String::find_previous_word(code, pos);
        if (!func_name)
        {
            assert(false);
            continue;
        }

        func_names.insert(*func_name);
        static constexpr auto ctx_declaration = "CoollabContext coollab_context, "sv;
        code.insert(pos + magic_comment.size(), ctx_declaration);

        pos += magic_comment.size() + ctx_declaration.size();
        pos = code.find(magic_comment, pos);
    }

    for (auto const& func_name : func_names)
        Cool::String::replace_all_beginnings_of_words_inplace(code, func_name + "(", func_name + "(coollab_context, "); // Will only replace in places where we call the function, and not where we declare the function because there there is our magic comment between the name of the function and the parenthesis.

    // Fixup the extra commas for functions that had no arguments initially
    Cool::String::replace_all_inplace(code, ", )", ")");
    Cool::String::replace_all_inplace(code, "(coollab_context, ()", "(coollab_context");
}

static void remove_extra_parentheses(std::string& code)
{
    Cool::String::replace_all_inplace(code, "(())", "()");
}

auto generate_shader_code(
    Cool::NodeId const&                              root_node_id,
    MaybeGenerateModule const&                       maybe_generate_module,
    FunctionSignature const&                         signature,
    ShaderCodeBits const&                            content,
    std::function<std::vector<std::string>()> const& get_module_textures_names,
    DataToGenerateShaderCode const&                  data
) -> tl::expected<std::string, std::string>
{
    auto       context            = CodeGenContext{data.nodes_graph, data.get_node_definition};
    auto const main_function_name = gen_desired_function(
        signature,
        root_node_id,
        context,
        maybe_generate_module
    );

    if (!main_function_name)
        return tl::make_unexpected(fmt::format("Failed to generate shader code:\n{}", main_function_name.error()));

    std::string modules_textures_uniforms;
    {
        auto const tex_names = get_module_textures_names();
        // TODO(WebGPU)
        // for (auto const& name : tex_names)
        //     modules_textures_uniforms += fmt::format("uniform sampler2D {};\n", name);
    }

    inject_context_argument_in_all_functions(context.code());
    remove_extra_parentheses(context.code());

    using fmt::literals::operator""_a;
    return fmt::format(
        FMT_COMPILE(R"glsl(
{in_version}
uniform float _delta_time;
uniform float _time;
uniform float _height;
uniform int   _last_midi_button_pressed;
uniform int   _last_last_midi_button_pressed;
uniform float _time_since_last_midi_button_pressed;
uniform float _audio_volume;
uniform mat3  _camera2D_transform;
uniform mat3  _camera2D_view;
uniform bool  _display_audio_spectrum_as_bars;
// group (aka "set") 0 is reserved for internal stuff from the FullscreenPipeline
layout(set=1, binding=0) uniform texture2D _previous_frame_texture;
layout(set=1, binding=1) uniform sampler _previous_frame_texture_sampler;
layout(set=1, binding=2) uniform texture2D mixbox_lut;      // The uniform must have this exact name that mixbox.glsl expects.
layout(set=1, binding=3) uniform sampler mixbox_lut_sampler;// The uniform must have this exact name that mixbox.glsl expects.
layout(set=1, binding=4)  readonly buffer _audio_spectrum_buffer {{ float _audio_spectrum[]; }} ;
layout(set=1, binding=5)  readonly buffer _audio_waveform_buffer {{ float _audio_waveform[]; }} ;
{modules_textures_uniforms}

#include "_COOL_RES_/shaders/shader-utils.glsl" 
#include "_ROOT_FOLDER_/res/mixbox/mixbox.glsl"
#include "_COOL_RES_/shaders/math.glsl"
#include "_COOL_RES_/shaders/rand.glsl"
#include "_COOL_RES_/shaders/color_conversions.glsl"
#include "_COOL_RES_/shaders/Texture.glsl"
#include "_COOL_RES_/shaders/camera.glsl"

vec2 to_view_space(vec2 uv)
{{
    vec3 p = _camera2D_view * vec3(uv, 1.);
    return p.xy / p.z;
}}

{in_before_main}
{output_indices_declarations}
{helper_code}
{main_function}
        )glsl"),
        "in_version"_a                  = content.version,
        "modules_textures_uniforms"_a   = modules_textures_uniforms,
        "in_before_main"_a              = content.before_main,
        "output_indices_declarations"_a = gen_all_output_indices_declarations(data.nodes_graph),
        "helper_code"_a                 = context.code(),
        "main_function"_a               = content.make_main(*main_function_name)
    );
}

} // namespace Lab