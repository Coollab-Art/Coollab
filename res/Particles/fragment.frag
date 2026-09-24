#version 430

in vec2      _varying_uv;
flat in vec4 _varying_color;
out vec4     _out_color;

void main()
{
    if (dot(_varying_uv - vec2(0.5), _varying_uv - vec2(0.5)) > 0.25)
        discard;
    _out_color = _varying_color;
}
