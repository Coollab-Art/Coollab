#version 430

in vec2  _varying_uv;
out vec4 _out_color;

uniform float u_fade;
uniform float u_fade_pow;

void main()
{
    float dist = distance(_varying_uv, vec2(0.5));
    if (dist > 0.5)
        discard;
    _out_color = vec4(
        vec3(1.) * pow(u_fade, u_fade_pow),
        1.
    );
}