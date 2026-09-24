
layout(location = 0) in vec2 _position;
layout(location = 1) in vec2 _uv;

// The particle attributes are read as *instanced* vertex attributes, straight out of the same
// buffers that the simulation compute shader writes to as SSBOs.
// We deliberately don't read them as SSBOs here: SSBO support in the vertex and fragment stages is
// optional in OpenGL 4.3 (GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS is allowed to be 0, only the compute
// stage is guaranteed), so a perfectly conformant driver can refuse to link such a shader.
// Instanced vertex attributes are guaranteed on any driver that can run the app at all.
#ifdef COOLLAB_PARTICLES_3D
layout(location = 2) in vec3 _particle_position;
#else
layout(location = 2) in vec2 _particle_position;
#endif
layout(location = 3) in float _particle_size;
layout(location = 4) in float _particle_lifetime;
layout(location = 5) in float _particle_lifetime_max;
layout(location = 6) in vec4  _particle_color;

out vec2      _varying_uv;
flat out vec4 _varying_color; // flat: the colour is per-particle, there is nothing to interpolate between the vertices of a given quad

#ifdef COOLLAB_PARTICLES_3D
uniform mat4 cool_camera_view;
#endif
uniform mat4 view_proj_matrix;

void main()
{
    // Always write every output, on every path: a shader output that is left unwritten has an
    // undefined value, and the fragment shader would then read it.
    _varying_uv    = _uv;
    _varying_color = _particle_color;

    if (_particle_lifetime <= 0 && _particle_lifetime_max > 0)
    {
        // Discard dead particles by collapsing the quad into a zero-area triangle.
        // w must be 1, not 0: with w == 0 the clip test -w <= x <= w passes trivially and the
        // perspective divide is a division by zero, which drivers handle differently (some discard
        // the primitive, some rasterize garbage).
        gl_Position = vec4(0., 0., 0., 1.);
        return;
    }

#ifdef COOLLAB_PARTICLES_3D
    vec3 particle_position = _particle_position;
    vec3 camera_right      = vec3(cool_camera_view[0][0], cool_camera_view[1][0], cool_camera_view[2][0]);
    vec3 camera_up         = vec3(cool_camera_view[0][1], cool_camera_view[1][1], cool_camera_view[2][1]);
#else
    vec3 particle_position = vec3(_particle_position, 0);
    vec3 camera_right      = vec3(1, 0, 0);
    vec3 camera_up         = vec3(0, 1, 0);
#endif
    gl_Position = view_proj_matrix
                  * vec4(
                      particle_position
                          + camera_right * _position.x * _particle_size
                          + camera_up * _position.y * _particle_size,
                      1.
                  );
}
