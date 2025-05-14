
layout(location = 0) in vec2 _position;
layout(location = 1) in vec2 _uv;

uniform vec2 u_position;
uniform mat3 view_proj_matrix;

uniform float u_size;

out vec2 _varying_uv;

void main()
{
    _varying_uv   = _uv;
    vec3 proj_pos = view_proj_matrix * vec3(u_size * _position + u_position, 1.);
    gl_Position   = vec4(proj_pos.xy, 0., proj_pos.z);
}