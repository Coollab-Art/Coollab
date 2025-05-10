#version 430

in vec2      _varying_uv;
flat in uint _particle_index;
out vec4     _out_color;

layout(std430, binding = 5) buffer _colors_buffer
{
    float _colors[];
};

void main()
{
    float dist = dot(_varying_uv - vec2(0.5), _varying_uv - vec2(0.5));
    float mmax = 0.25;
    if (dist > mmax)
        discard;
    _out_color = vec4(
        (1. - dist / mmax) * _colors[_particle_index * 4 + 0],
        (1. - dist / mmax) * _colors[_particle_index * 4 + 1],
        (1. - dist / mmax) * _colors[_particle_index * 4 + 2],
        _colors[_particle_index * 4 + 3]
    );
}