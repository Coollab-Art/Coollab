#version 430

layout(std430, binding = 1) buffer _densities_buffer
{
    float _densities[];
};

out vec4 out_Color;
layout(location = 0) in vec2 _uv;

uniform uvec2 _grid_size;
// uniform mat3  _camera2D_transform;
// uniform float _aspect_ratio;

void main()
{
    uvec2 gid = uvec2(_uv * _grid_size);
    float t   = _densities[gid.x + gid.y * _grid_size.x];
    out_Color = vec4(vec3(t), 1.);
}