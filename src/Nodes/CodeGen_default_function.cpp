#include "CodeGen_default_function.h"
#include <optional>
#include "CodeGenContext.h"
#include "CodeGen_implicit_conversion.h"
#include "FunctionSignature.h"
#include "PrimitiveType.h"
#include "fmt/compile.h"

namespace Lab {

static auto comma_separated(std::string const& str, size_t count)
    -> std::string
{
    std::string res;

    for (size_t i = 0; i < count; ++i)
    {
        res += str;
        if (i != count - 1)
            res += ", ";
    }

    if (count == 1)
        return res;
    else
        return fmt::format("({})", res);
}

static auto declare_inputs(std::string const& type, size_t arity)
    -> std::string
{
    auto res = std::string{};
    for (size_t i = 0; i < arity; ++i)
    {
        res += fmt::format("{} in{}", type, i);
        if (i != arity - 1)
            res += ", ";
    }
    return res;
}

static auto list_converted_inputs(std::string const& conversion, size_t arity)
{
    if (arity == 0)
        return fmt::format("{}()", conversion);

    auto res = std::string{};
    for (size_t i = 0; i < arity; ++i)
    {
        res += fmt::format("{}(in{})", conversion, i);
        if (i != arity - 1)
            res += ", ";
    }
    return res;
}

static auto gen_function_with_implicit_conversions(
    std::string const&         base_name,
    FunctionSignature          desired_signature,
    ImplicitConversions const& implicit_conversions,
    CodeGenContext&            context
) -> std::string
{
    using fmt::literals::operator""_a;

    auto const name = fmt::format("{}implicitly{}", base_name, to_string(desired_signature));
    return context.push_function({
        .name       = name,
        .definition = fmt::format(
            FMT_COMPILE(R"STR(
{output} {name}/*needs_coollab_context*/({inputs_declaration})
{{
    return {output_conversion}({base_name}({converted_inputs}));
}}
        )STR"),
            "name"_a               = name,
            "base_name"_a          = base_name,
            "inputs_declaration"_a = declare_inputs(glsl_type_as_string(desired_signature.from), desired_signature.arity),
            "output"_a             = glsl_type_as_string(desired_signature.to),
            "converted_inputs"_a   = list_converted_inputs(implicit_conversions.input.value_or(""), desired_signature.arity),
            "output_conversion"_a  = implicit_conversions.output.value_or("")
        ),
    });
}

static auto maybe_generate_default(FunctionSignature current_signature, std::string const& name, std::string const& implementation, FunctionSignature desired_signature, CodeGenContext& context)
    -> std::optional<std::string>
{
    auto const implicit_conversions = gen_implicit_conversions(current_signature, desired_signature, context);
    if (!implicit_conversions.both_exist())
        return std::nullopt;

    auto const base_name = context.push_function({
        .name       = name,
        .definition = implementation,
    });
    return gen_function_with_implicit_conversions(base_name, desired_signature, implicit_conversions, context);
}

auto gen_default_function(FunctionSignature signature, CodeGenContext& context)
    -> ExpectedFunctionName
{
    using fmt::literals::operator""_a;
    static constexpr std::string_view signed_to_float = "antialiased_step(sd)";
    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::SignedDistance, PrimitiveType::Float},
            "default_signed_distance_to_float", fmt::format(R"STR(
    float default_signed_distance_to_float(float sd)
    {{
        return {};
    }}
    )STR",
                                                            signed_to_float),
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::SignedDistance, PrimitiveType::sRGB},
            "default_signed_distance_to_sRGB", fmt::format(R"STR(
    vec3 default_signed_distance_to_sRGB(float sd)
    {{
        return vec3({});
    }}
    )STR",
                                                           signed_to_float),
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::UV, PrimitiveType::SignedDistance},
            "default_shape", R"STR(
    float default_shape(vec2 uv)
    {{
        return length(uv) - 0.5;
    }}
    )STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::Vec3, PrimitiveType::SignedDistance},
            "default_shape_3D", R"STR(
    float default_shape_3D(vec3 pos)
    {{
        return length(pos) - 1.;
    }}
    )STR",
            signature, context
        );
        if (func)
            return *func;
    }
    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::Float, PrimitiveType::Float},
            "default_1D_function", R"STR(
    float default_1D_function(float x)
    {{
        return sin(x*TAU*10.)*0.5+0.5;
    }}
    )STR",
            signature, context
        );
        if (func)
            return *func;
    }

    if (!is_greyscale(signature)) // Without this, Greyscale would match the Image signature and use this default function, but we want to use another default function for Greyscale (the one that comes after this).
    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::UV, PrimitiveType::sRGB},
            "default_image_srgb", R"STR(
vec3 default_image_srgb(vec2 uv)
{
    return vec3(saturate(uv), 0.);
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::UV, PrimitiveType::Float},
            "default_uv_to_float", R"STR(
float default_uv_to_float(vec2 uv)
{
    return uv.x * 0.5 / _aspect_ratio + 0.5;
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::Float, PrimitiveType::UV},
            "default_curve", R"STR(
vec2 default_curve(float t)
{
    float angle = TAU * t;
    return vec2(cos(angle), sin(angle));
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::Float, PrimitiveType::Vec3},
            "default_curve_3D", R"STR(
vec3 default_curve_3D(float t)
{
    float angle = TAU * t;
    return vec3(cos(angle), sin(angle), 0.);
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{PrimitiveType::UV, PrimitiveType::Vec2},
            "default_displacement_map", R"STR(
vec2 default_displacement_map(vec2 uv)
{
    return normalize(uv);
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    {
        auto const func = maybe_generate_default(
            FunctionSignature{.from = PrimitiveType::Oklab_PremultipliedA, .to = PrimitiveType::Oklab_PremultipliedA, .arity = 2},
            "default_blend_mode", R"STR(
vec4 default_blend_mode(vec4 over, vec4 under)
{
    // This is a over (aka Normal Blend Mode).
    return over + (1. - over.a) * under;
}
)STR",
            signature, context
        );
        if (func)
            return *func;
    }

    // No need to use maybe_generate_default here because we don't need to rely on default conversions (there is no default conversion with Void anyways)
    if (signature.from == PrimitiveType::Void)
    {
        if (signature.to == PrimitiveType::UV) // Special case for UVs; they are not really constant, they use the current uv map, to which we have applied all the uv transformations, starting from normalized_uv().
        {
            return context.push_function({
                .name       = "default_uv",
                .definition = R"STR(
vec2 default_uv/*needs_coollab_context*/()
{
    return coollab_context.uv;
}
)STR",
            });
        }
        if (signature.to == PrimitiveType::Particle2D) // Special case for Particle2D: we can get the current particle from the context
        {
            return context.push_function({
                .name       = "default_particle_2D",
                .definition = R"STR(
Particle2D default_particle_2D/*needs_coollab_context*/()
{
    return coollab_context.particle;
}
)STR",
            });
        }
        if (signature.to == PrimitiveType::Particle3D) // Special case for Particle3D: we can get the current particle from the context
        {
            return context.push_function({
                .name       = "default_particle_3D",
                .definition = R"STR(
Particle3D default_particle_3D/*needs_coollab_context*/()
{
    return coollab_context.particle;
}
)STR",
            });
        }
        auto const glsl_type = raw_glsl_type_as_string(signature.to);
        auto const name      = fmt::format("default_constant_{}", glsl_type); // Must always contain "default_constant" because of a HACK in gen_transformed_inputs() of CodeGen_desired_function_implementation.cpp
        return context.push_function({
            .name       = name,
            .definition = fmt::format(
                FMT_COMPILE(R"STR(
{type} {name}()
{{
    return {type}(0);
}}
)STR"),
                "type"_a = glsl_type,
                "name"_a = name
            ),
        });
    }

    return tl::make_unexpected(fmt::format(
        "Could not generate a default function from {} to {}.",
        comma_separated(cpp_type_as_string(signature.from), signature.arity), cpp_type_as_string(signature.to)
    ));
}

} // namespace Lab
