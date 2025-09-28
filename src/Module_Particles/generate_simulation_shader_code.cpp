#include "generate_simulation_shader_code.h"
#include <Nodes/PrimitiveType.h>
#include "Module/ShaderBased/generate_shader_code.hpp"
#include "Nodes/Node.h"
#include "simulation_code_gen_helpers.h"

namespace Lab {

auto generate_simulation_shader_code(
    Cool::NodeId const&             root_node_id,
    Cool::NodeId&                   id_of_node_storing_particles_count,
    int                             dimension,
    DataToGenerateShaderCode const& data,
    MaybeGenerateModule const&      maybe_generate_module
) -> tl::expected<std::string, std::string>
{
    using fmt::literals::operator""_a;
    auto const content = ShaderCodeBits{
        .version     = "",
        .before_main = fmt::format(FMT_COMPILE(R"glsl(
#define COOLLAB_PARTICLES_{N}D

uniform bool _force_init_particles;

layout(std430, binding = 0) buffer _positions_buffer
{{
    float _positions[];
}};
layout(std430, binding = 1) buffer _velocities_buffer
{{
    float _velocities[];
}};
layout(std430, binding = 2) buffer _sizes_buffer
{{
    float _sizes[];
}};
layout(std430, binding = 3) buffer _lifetimes_buffer
{{
    float _lifetimes[];
}};
layout(std430, binding = 4) buffer _lifetime_maxs_buffer
{{
    float _lifetime_maxs[];
}};
layout(std430, binding = 5) buffer _colors_buffer
{{
    float _colors[];
}};

struct Particle{N}D
{{
    vec{N} position;
    vec{N} velocity;
    vec{N} acceleration;
    float  size;
    float  lifetime;
    float  lifetime_max;
    vec4   color;
    uint   index;
    bool   needs_init;
}};

struct CoollabContext
{{
    vec2 uv;
    Particle{N}D particle;
}};

uint particles_count()
{{
   return DispatchSize.x;
}}

)glsl"),
                                   "N"_a = dimension),
        .make_main   = [&](std::string const& main_function_name) {
            return fmt::format(
                R"glsl(
void cool_main()
{{
    uint gid = gl_GlobalInvocationID.x;
    Particle{N}D particle;
    particle.position     = {position_b2v};
    particle.velocity     = {velocity_b2v};
    particle.acceleration = vec{N}(0.);
    particle.size         = _sizes[gid];
    particle.lifetime     = _lifetimes[gid];
    particle.lifetime_max = _lifetime_maxs[gid];
    particle.color.x      = _colors[gid * 4 + 0];
    particle.color.y      = _colors[gid * 4 + 1];
    particle.color.z      = _colors[gid * 4 + 2];
    particle.color.w      = _colors[gid * 4 + 3];
    particle.index        = gid;

    particle.needs_init = _force_init_particles || (particle.lifetime >= 0. && particle.lifetime - _delta_time < 0.);
    particle.lifetime -= _delta_time;
    if (particle.needs_init)
    {{ // Default initialization
#ifdef COOLLAB_PARTICLES_2D
        particle.position = hash_0_to_1_2D_to_2D(vec2(gid, 0)) * 2. - 1.;
        particle.velocity = hash_0_to_1_2D_to_2D(vec2(gid, 1)) * 2. - 1.;
#else
        particle.position = hash_0_to_1_2D_to_3D(vec2(gid, 0)) * 2. - 1.;
        particle.velocity = hash_0_to_1_2D_to_3D(vec2(gid, 1)) * 2. - 1.;
#endif
        particle.size  = 0.01;
        particle.color = vec4(1.0);
        particle.lifetime_max = -1.; // To be sure that the vertex shader will show the particles
    }}

    CoollabContext coollab_context;
    coollab_context.uv = particle.position.xy;
    coollab_context.particle = particle;

    particle = {main_function_name}(coollab_context, particle);

    particle.velocity += particle.acceleration * _delta_time;
    particle.position += particle.velocity * _delta_time; // TODO(Particles) proper integration method

    {position_v2b}
    {velocity_v2b}
    _sizes[gid]          = particle.size;
    _lifetimes[gid]      = particle.lifetime;
    _lifetime_maxs[gid]  = particle.lifetime_max;
    _colors[gid * 4 + 0] = particle.color.x;
    _colors[gid * 4 + 1] = particle.color.y;
    _colors[gid * 4 + 2] = particle.color.z;
    _colors[gid * 4 + 3] = particle.color.w;
}})glsl",
                "main_function_name"_a = main_function_name,
                "position_b2v"_a       = buffer_to_vec(dimension, "_positions", "gid"),
                "velocity_b2v"_a       = buffer_to_vec(dimension, "_velocities", "gid"),
                "position_v2b"_a       = vec_to_buffer(dimension, "_positions", "gid", "particle.position"),
                "velocity_v2b"_a       = vec_to_buffer(dimension, "_velocities", "gid", "particle.velocity"),
                "N"_a                  = dimension
            );
        },
    };

    auto const main_function_signature = FunctionSignature{
        .from  = primitive_type_particle(dimension),
        .to    = primitive_type_particle(dimension),
        .arity = 1,
    };

    auto const node_definition_callback = [&](Cool::NodeId const& node_id, NodeDefinition const& node_definition) {
        auto const* maybe_node = data.nodes_graph.try_get_node<Node>(node_id);
        if (maybe_node && maybe_node->particles_count().has_value())
            id_of_node_storing_particles_count = node_id;
        return maybe_generate_module(node_id, node_definition);
    };

    return generate_shader_code(
        root_node_id,
        node_definition_callback,
        main_function_signature,
        content,
        []() { return std::vector<std::string>{}; },
        data
    );
}

} // namespace Lab
