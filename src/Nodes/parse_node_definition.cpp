#include "parse_node_definition.h"
#include <Cool/Expected/RETURN_IF_ERROR.h>
#include <Cool/Expected/RETURN_IF_UNEXPECTED.h>
#include <Cool/RegExp/RegExp.h>
#include <Cool/String/String.h>
#include <exception>
#include <string_view>
#include "Cool/ColorSpaces/ColorAndAlphaSpace.h"
#include "Cool/ColorSpaces/ColorSpace.h"
#include "Cool/Dependencies/SharedVariableDefinition.h"
#include "Cool/File/File.h"
#include "Cool/type_from_string/type_from_string.h"
#include "Debug/DebugOptions.h"
#include "FunctionSignature.h"
#include "NodeDefinition.h"
#include "PrimitiveType.h"
#include "fixup_node_definition.h"
#include "stringify/stringify.hpp"
#include "tl/expected.hpp"

namespace Lab {

static auto decompose_signature_string(std::string const& text, size_t end_of_function_declaration)
    -> tl::expected<FunctionSignatureAsString, std::string>
{
    auto args_pos = Cool::String::rfind_matching_pair({
        .text    = text,
        .offset  = end_of_function_declaration,
        .opening = '(',
        .closing = ')',
    });
    if (!args_pos)
        return tl::make_unexpected("Did not find the arguments of the main function. You are missing a parenthesis.");
    args_pos->first += 1; // Skip the first parenthesis

    auto const name_pos = Cool::String::find_previous_word_position(text, args_pos->first);
    if (!name_pos)
        return tl::make_unexpected("Did not find the name of the main function.");

    auto const return_type_pos = Cool::String::find_previous_word_position(text, name_pos->first);
    if (!return_type_pos)
        return tl::make_unexpected("Did not find the return type of the main function.");

    return FunctionSignatureAsString{
        .return_type    = Cool::String::substring(text, *return_type_pos),
        .name           = Cool::String::substring(text, *name_pos),
        .arguments_list = Cool::String::substring(text, *args_pos),
    };
}

static auto parse_primitive_type(std::string const& str) // NOLINT(readability-function-cognitive-complexity)
    -> tl::expected<PrimitiveType, std::string>
{
#include "generated/parse_primitive_type.inl"

    return tl::make_unexpected(fmt::format(
        "Unknown type '{}'. The available types are:\n{}",
        str,
#include "generated/string_listing_the_parsed_types.inl"
    ));
}

struct Argument {
    PrimitiveType type;
    std::string   name;
};

static auto parse_arguments(std::string const& arguments_list)
    -> tl::expected<std::vector<Argument>, std::string>
{
    auto args = std::vector<Argument>{};

    auto type_pos = Cool::String::find_next_word_position(arguments_list, 0);
    while (type_pos)
    {
        auto const type = parse_primitive_type(Cool::String::substring(arguments_list, *type_pos));
        RETURN_IF_UNEXPECTED(type);

        auto const name_pos = Cool::String::find_next_word_position(arguments_list, type_pos->second);
        if (!name_pos)
            return tl::make_unexpected("Missing argument name in main function.");

        args.push_back(Argument{
            .type = *type,
            .name = Cool::String::substring(arguments_list, *name_pos),
        });

        type_pos = Cool::String::find_next_word_position(arguments_list, name_pos->second);
    }

    return args;
}

static auto gen_arguments_list(std::vector<Argument> const& args)
    -> std::string
{
    using fmt::literals::operator""_a;

    std::string res{};
    for (size_t i = 0; i < args.size(); ++i)
    {
        res += fmt::format(
            FMT_COMPILE("{type} {name}"),
            "type"_a = glsl_type_as_string(args[i].type),
            "name"_a = args[i].name
        );
        if (i != args.size() - 1)
            res += ", ";
    }

    return res;
}

static auto make_main_function(std::string const& text, std::pair<size_t, size_t> brackets_pos, std::string const& name)
    -> tl::expected<MainFunction, std::string>
{
    auto main = MainFunction{};

    main.body = Cool::String::substring(text, brackets_pos.first + 1, brackets_pos.second - 1);

    auto const signature_as_string = decompose_signature_string(text, brackets_pos.first);
    RETURN_IF_UNEXPECTED(signature_as_string);
    main.signature_as_string = *signature_as_string;

    main.signature_as_string.name = name; // Change name, we don't want it to be "main", but the name of the node

    { // Parse return type
        auto const return_type = parse_primitive_type(main.signature_as_string.return_type);
        RETURN_IF_UNEXPECTED(return_type);
        main.signature.to                    = *return_type;
        main.signature_as_string.return_type = glsl_type_as_string(*return_type); // Change the return type to a regular glsl type so that it is valid when copy-pasted into shader code.
    }

    { // Parse arguments list
        auto const arguments = parse_arguments(main.signature_as_string.arguments_list);
        RETURN_IF_UNEXPECTED(arguments);

        main.signature.arity = arguments->size();
        main.signature.from  = arguments->empty()
                                   ? PrimitiveType::Void
                                   : (*arguments)[0].type;

        main.signature_as_string.arguments_list = gen_arguments_list(*arguments); // Change arguments list so that it only uses regular glsl types, so that it is valid when copy-pasted into shader code.

        for (auto const& arg : *arguments)
        {
            main.argument_names.push_back(arg.name);
            // Check that we only have one argument type
            if (arg.type != (*arguments)[0].type)
            {
                return tl::make_unexpected(fmt::format(
                    "The main function cannot have different arguments types. Found {} and {}.\nIf you need more arguments, consider using an INPUT instead.",
                    cpp_type_as_string((*arguments)[0].type),
                    cpp_type_as_string(arg.type)
                ));
            }
        }
    }

    return main;
}

static auto find_main_function(std::string& text, NodeDefinition_Data& res, std::filesystem::path const& filepath)
    -> std::optional<std::string>
{
    auto const main_func_pos = Cool::String::find_word("main", text); // TODO(NodesParsing) Problem, e.g. if an INPUT name has main in it ('Some main thing'), of a file name in an include has "main".
    if (main_func_pos == std::string_view::npos)
        return "Missing a main function.";

    auto const brackets_pos = Cool::String::find_matching_pair({
        .text    = text,
        .offset  = main_func_pos,
        .opening = '{',
        .closing = '}',
    });
    if (!brackets_pos)
        return "Main function has no body.";

    auto const main_function = make_main_function(text, *brackets_pos, filepath.stem().string());
    if (!main_function)
        return main_function.error();
    res.main_function = *main_function;

    auto const main_func_type_pos = Cool::String::find_previous_word_position(text, main_func_pos);
    if (!main_func_type_pos)
        return "Missing a return type before the main function.";
    Cool::String::remove_substring(text, main_func_type_pos->first, brackets_pos->second + 1);

    return std::nullopt;
}

static auto parse_input_function_signature(std::vector<std::string> const& type_words)
    -> tl::expected<FunctionSignature, std::string>
{
    assert(type_words.size() >= 2);

    auto const return_type = parse_primitive_type(type_words.back());
    RETURN_IF_UNEXPECTED(return_type);

    auto const input_type = parse_primitive_type(type_words.front());
    RETURN_IF_UNEXPECTED(input_type);

    // Error checking
    if (input_type == PrimitiveType::Void)
        return tl::make_unexpected("'Void' is not allowed as an argument type for INPUT functions.");
    for (size_t i = 1; i < type_words.size() - 1; ++i)
    {
        auto const other_input_type = parse_primitive_type(type_words[i]);
        if (!other_input_type)
            return tl::make_unexpected(other_input_type.error());
        if (*input_type != *other_input_type)
            return tl::make_unexpected(fmt::format(
                "INPUT functions cannot have different arguments types. Found {} and {}.",
                cpp_type_as_string(*input_type),
                cpp_type_as_string(*other_input_type)
            ));
    }

    return FunctionSignature{
        .from  = *input_type,
        .to    = *return_type,
        .arity = type_words.size() - 1,
    };
}

template<typename T>
static auto make_shared_var_definition(std::string const& name, std::string const& type) -> Cool::AnySharedVariableDefinition
{
    auto def = Cool::SharedVariableDefinition<T>{{name}};

    if constexpr (std::is_same_v<T, Cool::Color>)
    {
        def.desired_color_space = static_cast<int>(Cool::parse_color_space(type));
    }
    else if constexpr (std::is_same_v<T, Cool::ColorAndAlpha>)
    {
        def.desired_color_space = static_cast<int>(Cool::parse_color_and_alpha_space(type));
    }

    return def;
}

static auto parse_input_value(std::string const& type_as_string, std::string const& name, NodeDefinition_Data& res)
    -> std::optional<std::string>
{
    try
    {
        res.input_values.emplace_back(COOL_TFS_EVALUATE_FUNCTION_TEMPLATE(
            make_shared_var_definition,
            type_as_string,
            Cool::AnySharedVariableDefinition,
            (name, type_as_string)
        ));
    }
    catch (std::exception const& e)
    {
        return e.what();
    }

    return std::nullopt;
}

static auto parse_input_function(std::vector<std::string> const& type_words, std::string const& name, NodeDefinition_Data& res)
    -> std::optional<std::string>
{
    auto const signature = parse_input_function_signature(type_words);
    if (!signature)
        return signature.error();

    res.input_functions.emplace_back(NodeInputDefinition_Data{
        .name      = name,
        .signature = *signature,
    });

    return std::nullopt;
}

template<typename Callback>
static auto find_declaration(std::string& text, std::string_view keyword, Callback const& handle_type_words)
    -> std::optional<std::string>
{
    size_t offset = text.find(keyword);
    while (offset != std::string::npos)
    {
        auto const start_of_declaration = offset;
        offset += keyword.length();
        auto const end_of_line = text.find(';', offset);

        auto const error_message = [&](std::string_view str) {
            return fmt::format("Invalid {} declaration: {}. While reading:\n{}", keyword, str, Cool::String::substring(text, offset - keyword.length(), end_of_line != std::string::npos ? end_of_line + 1 : text.length()));
        };

        if (end_of_line == std::string::npos)
            return error_message("missing semicolon (;)");

        auto const name_pos = Cool::String::find_matching_pair({
            .text    = text,
            .offset  = offset,
            .opening = '\'',
            .closing = '\'',
        });
        if (!name_pos || name_pos->second > end_of_line)
            return error_message("missing name. A name must start and end with single quotes (')");

        auto const type_words = Cool::String::all_words(Cool::String::substring(text, offset, name_pos->first));

        if (type_words.empty())
            return error_message("missing type");

        {
            auto const name = Cool::String::substring(text, name_pos->first, name_pos->second + 1);

            auto const err = handle_type_words(type_words, name);
            if (err)
                return error_message(fmt::format("{}\n", *err));
        }
        Cool::String::remove_substring(text, start_of_declaration, end_of_line + 1);
        offset = text.find(keyword, start_of_declaration);
    }

    return std::nullopt;
}

static auto find_inputs(std::string& text, NodeDefinition_Data& res)
    -> std::optional<std::string>
{
    return find_declaration(
        text,
        "INPUT",
        [&](std::vector<std::string> const& type_words, std::string const& name) -> std::optional<std::string> {
            return type_words.size() == 1
                       ? parse_input_value(type_words[0], name, res)
                       : parse_input_function(type_words, name, res);
        }
    );
}

static auto find_outputs(std::string& text, NodeDefinition_Data& res)
    -> std::optional<std::string>
{
    return find_declaration(
        text,
        "OUTPUT",
        [&](std::vector<std::string> const& type_words, std::string const& name) -> std::optional<std::string> {
            if (type_words.size() > 1)
                return fmt::format("too many words; expected a type declaration, got {}", Cool::stringify(type_words));
            if (type_words[0] != "float")
                return fmt::format("invalid type. 'float' is the only allowed OUTPUT type. Found '{}'", type_words[0]);
            res.output_indices.emplace_back(name);
            return std::nullopt;
        }
    );
}

template<typename T>
static void maybe_set_input_description(std::string const& input_name, Cool::SharedVariableDefinition<T>& def, std::string const& description)
{
    if (def.var_data.name == input_name)
        def.description = description;
}

static void set_input_description(NodeDefinition_Data& res, std::string const& input_name, std::string const& input_description)
{
    for (auto& input : res.input_values)
    {
        std::visit([&](auto&& input) { maybe_set_input_description(input_name, input, input_description); }, input);
    }

    for (auto& input : res.input_functions)
    {
        if ("'" + input.name() + "'" == input_name)
            input.set_description(input_description);
    }
}

static void find_inputs_descriptions(std::string const& text, NodeDefinition_Data& res)
{
    size_t desc_offset = 0;
    size_t name_offset = 0;

    auto pos_of_the_input_name = std::make_optional<std::pair<size_t, size_t>>(); // Dummy value. We just need it to not be nullopt in order to enter the while loop, but it will be properly computed at the very beginning of that while loop.
    while (pos_of_the_input_name)
    {
        // Update positions
        pos_of_the_input_name = Cool::String::find_matching_pair({
            .text    = text,
            .offset  = name_offset,
            .opening = '\'',
            .closing = '\'',
        });
        if (!pos_of_the_input_name)
            break;

        auto const pos_of_text_after_the_input_declaration = Cool::String::find_matching_pair({
            .text    = text,
            .offset  = desc_offset,
            .opening = ';',
            .closing = '\n',
        });
        if (!pos_of_text_after_the_input_declaration)
            break;

        auto const move_to_next_description = [&]() {
            name_offset = pos_of_the_input_name->second + 1;
            desc_offset = pos_of_text_after_the_input_declaration->second + 1;
        };

        auto const text_after_the_input_declaration = Cool::String::substring(text, *pos_of_text_after_the_input_declaration);

        auto const triple_slash_pos = text_after_the_input_declaration.find("///");
        if (triple_slash_pos == std::string::npos)
        {
            move_to_next_description();
            continue;
        }

        auto const input_description = Cool::String::substring(
            text_after_the_input_declaration,
            triple_slash_pos + 3,
            text_after_the_input_declaration.size()
        );

        pos_of_the_input_name->second = pos_of_the_input_name->second + 1; // Increase by 1 to include the last single quote (')
        std::string const input_name  = Cool::String::substring(text, *pos_of_the_input_name);

        set_input_description(res, input_name, input_description);

        move_to_next_description();
    }
}

static auto input_fix_artifacts()
{
    return Cool::SharedVariableDefinition<float>{
        .var_data = Cool::VariableData<float>{
            .name     = "'Fix Artifacts'",
            .value    = 0.f,
            .metadata = Cool::VariableMetadata<float>{
                .bounds = {
                    .min           = 0.f,
                    .max           = 0.999f,
                    .has_min_bound = true,
                    .has_max_bound = true,
                    .drag_speed    = 0.01f,
                    .use_slider    = true,
                },
            },
        },
        .description = "Increase the value to fix glitches and holes in the shape. But note that higher values are slower to render.",
    };
}

static auto parse_special_coollab_syntax_and_remove_it(
    std::string const&           text,
    std::string&                 text_without_comments,
    std::filesystem::path const& filepath,
    NodeDefinition_Data&         def
) -> std::optional<std::string>
{
    bool const fix_artifacts = text_without_comments.find("CLB_FIX_ARTIFACTS") != std::string::npos;
    if (fix_artifacts)
        Cool::String::replace_all_words_inplace(text_without_comments, "CLB_FIX_ARTIFACTS", "(1. - 'Fix Artifacts') * ");

    RETURN_IF_ERR(find_main_function(text_without_comments, def, filepath));
    RETURN_IF_ERR(find_inputs(text_without_comments, def));
    RETURN_IF_ERR(find_outputs(text_without_comments, def));
    find_inputs_descriptions(text, def); // Must be done after finding the inputs. Must work on the text WITH comments because the descriptions are inside comments.

    if (fix_artifacts) // Done after finding the inputs, because we want this to be the last input
        def.input_values.emplace_back(input_fix_artifacts());

    return {};
}

enum class ScopeKind {
    Parenthesis,
    CurlyBracket,
    SquareBracket,
};
void for_each_scope_kind(std::function<void(ScopeKind)> const& callback)
{
    callback(ScopeKind::Parenthesis);
    callback(ScopeKind::CurlyBracket);
    callback(ScopeKind::SquareBracket);
}

static auto opening_char(ScopeKind scope) -> char
{
    switch (scope)
    {
    case ScopeKind::Parenthesis:
        return '(';
    case ScopeKind::CurlyBracket:
        return '{';
    case ScopeKind::SquareBracket:
        return '[';
    }
    assert(false);
    return '0';
}
static auto closing_char(ScopeKind scope) -> char
{
    switch (scope)
    {
    case ScopeKind::Parenthesis:
        return ')';
    case ScopeKind::CurlyBracket:
        return '}';
    case ScopeKind::SquareBracket:
        return ']';
    }
    assert(false);
    return '0';
}

static auto is_keyword(std::string const& word)
{
    static const auto glsl_keywords = std::set<std::string>{
        "const", "uniform", "buffer", "shared", "attribute", "varying",
        "coherent", "volatile", "restrict", "readonly", "writeonly",
        "atomic_uint",
        "layout",
        "centroid", "flat", "smooth", "noperspective",
        "patch", "sample",
        "invariant", "precise",
        "break", "continue", "do", "for", "while", "switch", "case", "default",
        "if", "else",
        "subroutine",
        "in", "out", "inout",
        "int", "void", "bool", "true", "false", "float", "double",
        "discard", "return",
        "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4",
        "uint", "uvec2", "uvec3", "uvec4",
        "dvec2", "dvec3", "dvec4",
        "mat2", "mat3", "mat4",
        "mat2x2", "mat2x3", "mat2x4",
        "mat3x2", "mat3x3", "mat3x4",
        "mat4x2", "mat4x3", "mat4x4",
        "dmat2", "dmat3", "dmat4",
        "dmat2x2", "dmat2x3", "dmat2x4",
        "dmat3x2", "dmat3x3", "dmat3x4",
        "dmat4x2", "dmat4x3", "dmat4x4",
        "lowp", "mediump", "highp", "precision",
        "sampler1D", "sampler1DShadow", "sampler1DArray", "sampler1DArrayShadow",
        "isampler1D", "isampler1DArray", "usampler1D", "usampler1DArray",
        "sampler2D", "sampler2DShadow", "sampler2DArray", "sampler2DArrayShadow",
        "isampler2D", "isampler2DArray", "usampler2D", "usampler2DArray",
        "sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect",
        "sampler2DMS", "isampler2DMS", "usampler2DMS",
        "sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray",
        "sampler3D", "isampler3D", "usampler3D",
        "samplerCube", "samplerCubeShadow", "isamplerCube", "usamplerCube",
        "samplerCubeArray", "samplerCubeArrayShadow",
        "isamplerCubeArray", "usamplerCubeArray",
        "samplerBuffer", "isamplerBuffer", "usamplerBuffer",
        "image1D", "iimage1D", "uimage1D",
        "image1DArray", "iimage1DArray", "uimage1DArray",
        "image2D", "iimage2D", "uimage2D",
        "image2DArray", "iimage2DArray", "uimage2DArray",
        "image2DRect", "iimage2DRect", "uimage2DRect",
        "image2DMS", "iimage2DMS", "uimage2DMS",
        "image2DMSArray", "iimage2DMSArray", "uimage2DMSArray",
        "image3D", "iimage3D", "uimage3D",
        "imageCube", "iimageCube", "uimageCube",
        "imageCubeArray", "iimageCubeArray", "uimageCubeArray",
        "imageBuffer", "iimageBuffer", "uimageBuffer",
        "struct"
    };

    return glsl_keywords.find(word) != glsl_keywords.end();
}

static auto is_word_separator(char c)
{
    return Cool::String::default_word_delimiters.find(c) != std::string_view::npos
           && c != '#';
}

static auto is_digit(char c) -> bool
{
    return '0' <= c && c <= '9';
}

auto find_names_declared_in_global_scope(std::string& text, NodeDefinition_Data& def) -> std::optional<std::string> // NOLINT(function-cognitive-complexity)
{
    auto previous_word = ""s;
    auto current_word  = ""s;
    auto scopes_stack  = std::vector<ScopeKind>{};
    int  line_number   = 1; // TODO(NodesParsing) For the line number to match, we would need to keep the lines containing special Coollab syntax, and the comments.
    bool is_in_number  = false;
    bool is_in_macro   = false;

    auto const is_in_global_scope = [&]() {
        return scopes_stack.empty();
    };

    for (size_t index = 0; index < text.size(); ++index)
    {
        char const c = text[index];
        if (!is_word_separator(c))
        {
            if (current_word.empty() && is_digit(c))
                is_in_number = true;
            if (c == '#')
                is_in_macro = true;
            current_word += c;
            continue;
        }

        bool is_function_declaration{false};
        if (is_in_global_scope() && !current_word.empty() && !is_keyword(current_word) && !is_in_number && (!is_in_macro || previous_word == "#define"))
        {
            def.names_in_global_scope.push_back(current_word);
            is_function_declaration = c == '(' && !is_in_macro;
        }
        previous_word = current_word;
        current_word  = "";
        is_in_number  = false;
        if (c == '\n')
        {
            line_number += 1;
            if (index == 0 || text[index - 1] != '\\')
                is_in_macro = false;
        }

        try
        {
            for_each_scope_kind([&](ScopeKind scope) {
                if (c == opening_char(scope))
                {
                    scopes_stack.push_back(scope);
                }
                if (c == closing_char(scope))
                {
                    if (scopes_stack.empty())
                        throw fmt::format("Mismatched brackets: found '{}' on line {}, but there is no opening '{}'.", c, line_number, opening_char(scope)); // NOLINT(*exception-baseclass)
                    if (scopes_stack.back() != scope)
                        throw fmt::format("Mismatched brackets: found '{}' on line {}, expected '{}'.", c, line_number, closing_char(scopes_stack.back())); // NOLINT(*exception-baseclass)
                    scopes_stack.pop_back();
                }
            });
        }
        catch (std::string const& msg)
        {
            return msg;
        }
        if (is_function_declaration)
        {
            static constexpr auto magic_comment = "/*needs_coollab_context*/"sv;
            text.insert(index, magic_comment);
            index += magic_comment.size();
        }
    }

    if (!scopes_stack.empty())
        return fmt::format("Mismatched brackets: missing a closing '{}'", closing_char(scopes_stack.back()));
    return {};
}

static auto presets_paths(std::filesystem::path path) -> Cool::PresetsPaths
{
    path += ".presets.json";
#if DEBUG
    return Cool::PresetsPaths{
        .user_defined_presets = std::move(path),
        .default_presets      = "",
    };
#else
    return Cool::PresetsPaths{
        .user_defined_presets = Cool::Path::user_data() / Cool::File::relative(path, Cool::Path::root()), // Convert a path relative to root() into a path relative to user_data()
        .default_presets = path,
    };
#endif
}

auto parse_node_definition(std::filesystem::path const& filepath, std::string const& text)
    -> tl::expected<NodeDefinition, std::string>
{
    auto def = NodeDefinition_Data{};
    if (DebugOptions::log_when_parsing_node_definition())
        Cool::Log::info("Nodes", fmt::format("Parsing node definition from \"{}\"", Cool::File::weakly_canonical(filepath)));

    auto text_without_comments = Cool::String::remove_comments(text);
    RETURN_IF_ERROR(parse_special_coollab_syntax_and_remove_it(text, text_without_comments, filepath, def)); // Leaves only regular glsl code
    RETURN_IF_ERROR(find_names_declared_in_global_scope(text_without_comments, def));                        // That we can then check for all symbols declared in global scope (functions, structs, constants, #define, etc.)

    def.helper_glsl_code = text_without_comments; // Must be done after it has been modified by parse_special_coollab_syntax_and_remove_it() and find_names_declared_in_global_scope()
    fixup_node_definition(def);

    return NodeDefinition::make(def, presets_paths(filepath));
}

} // namespace Lab