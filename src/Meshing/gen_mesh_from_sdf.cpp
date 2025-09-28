#include "gen_mesh_from_sdf.hpp"
#include "Cool/Gpu/OpenGL/ComputeShader.h"
#include "Cool/Gpu/OpenGL/SSBO.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "MeshingSettings.hpp"
#include "Module/ModuleDependencies.h"
#include "Module/ShaderBased/DataToGenerateShaderCode.hpp"
#include "Module/ShaderBased/generate_shader_code.hpp"
#include "Module/ShaderBased/set_uniforms_for_shader_based_module.hpp"
#include "glm/gtx/component_wise.hpp"
#include "marching_cubes.hpp"

namespace Lab {

static auto generate_compute_shader(std::string const& shader_code) -> tl::expected<Cool::OpenGL::ComputeShader, Cool::ErrorMessage>
{
    try
    {
        return Cool::OpenGL::ComputeShader{64, shader_code};
    }
    catch (Cool::Exception const& e)
    {
        return tl::make_unexpected(e.error_message());
    }
}

static auto generate_meshing_shader_code(
    Cool::NodeId const&             root_node_id,
    DataToGenerateShaderCode const& data_to_generate_shader_code
) -> tl::expected<std::string, std::string>
{
    using fmt::literals::operator""_a;
    auto const content = ShaderCodeBits{
        .version     = "",
        .before_main = R"glsl(
uniform float _step_size;
uniform float _box_size;

layout(std430, binding = 0) buffer _signed_distance_field_buffer
{
    float _signed_distance_field[];
};

struct CoollabContext
{
    vec2 uv;
};
        )glsl",
        .make_main   = [](std::string const& main_function_name) {
            return fmt::format(FMT_COMPILE(R"glsl(
void cool_main()
{{
    uint gid = gl_GlobalInvocationID.x +
               gl_GlobalInvocationID.y * DispatchSize.x + 
               gl_GlobalInvocationID.z * DispatchSize.y * DispatchSize.x;

    vec3 pos = vec3(-_box_size / 2. + gl_GlobalInvocationID * _step_size);

    CoollabContext dummy_coollab_context;
    dummy_coollab_context.uv = vec2(0);
    _signed_distance_field[gid] = {main_function_name}(dummy_coollab_context, pos);
}}
                )glsl"),
                                 "main_function_name"_a = main_function_name);
        },
    };

    auto const main_function_signature = FunctionSignature{
        .from  = PrimitiveType::Vec3,
        .to    = PrimitiveType::SignedDistance,
        .arity = 1,
    };

    return generate_shader_code(
        root_node_id,
        [](auto&&, auto&&) -> std::optional<std::string> { return std::nullopt; },
        main_function_signature,
        content,
        []() {
            return std::vector<std::string>{}; /*TODO(Module) Pass the textures of the modules used by this shader*/
        },
        data_to_generate_shader_code
    );
}

auto gen_mesh_from_sdf(
    Cool::NodeId const&                         main_node_id,
    MeshingSettings const&                      meshing_settings,
    DataToPassToShader const&                   data_to_pass_to_shader,
    DataToGenerateShaderCode const&             data_to_generate_shader_code,
    std::vector<std::shared_ptr<Module>> const& module_dependencies
) -> std::optional<Cool::Mesh>
{
    if constexpr (COOL_OPENGL_VERSION < 430)
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Warning,
            .title   = "3D Model Export",
            .content = "OpenGL version is lower than 430. Meshing need compute shaders, which are not supported by this version.",
        });
        return std::nullopt;
    }

    auto sdf_ssbo = Cool::SSBO<float>{0 /*binding_id*/};
    sdf_ssbo.bind();
    size_t const ssbo_size{glm::compMul(meshing_settings.samples_count)};
    sdf_ssbo.upload_data(ssbo_size, nullptr);

    auto const shader_code{generate_meshing_shader_code(main_node_id, data_to_generate_shader_code)};

    if (!shader_code)
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Error,
            .title   = "3D Model Export",
            .content = fmt::format("Unable to generate the shader code:\n{}", shader_code.error()),
        });
        return std::nullopt;
    }

    auto meshing_compute_shader{generate_compute_shader(*shader_code)};

    if (!meshing_compute_shader)
    {
        assert(meshing_compute_shader.error().clipboard_contents.empty() && "Ignoring some clipboard contents");
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Error,
            .title   = "3D Model Export",
            .content = fmt::format("Unable to compile the compute shader:\n{}", meshing_compute_shader.error().message),
        });
        return std::nullopt;
    }

    meshing_compute_shader->bind();
    // TODO(Meshing) Handle boxes with different sizes on each axis
    meshing_compute_shader->set_uniform("_box_size", meshing_settings.box_size.x);
    meshing_compute_shader->set_uniform("_step_size", meshing_settings.step_size().x);

    {
        auto dependencies            = ModuleDependencies{};
        auto nodes_that_we_depend_on = std::vector<Cool::NodeId>{}; //  TODO(Meshing) Properly compute the nodes that we depend on
        for (auto const& [id, _] : data_to_pass_to_shader.nodes_graph.nodes())
            nodes_that_we_depend_on.emplace_back(id);
        update_dependencies_from_shader_code(dependencies, *shader_code);
        update_dependencies_from_nodes_graph(dependencies, data_to_pass_to_shader.nodes_graph, nodes_that_we_depend_on);
        set_uniforms_for_shader_based_module(*meshing_compute_shader, dependencies, data_to_pass_to_shader, module_dependencies, nodes_that_we_depend_on);
    }

    meshing_compute_shader->compute(meshing_settings.samples_count);

    // CPU get data back
    auto sdf_samples = std::vector<float>(ssbo_size);
    sdf_ssbo.download_data(sdf_samples);
    return gen_mesh_with_marching_cubes(sdf_samples, meshing_settings);
}

} // namespace Lab