#version 410

uniform float _time;

out vec4 out_Color;

// #include "_ROOT_FOLDER_/res/shader-lib/normalized_uv.glsl"
// #include "_ROOT_FOLDER_/res/shader-lib/image.glsl"
// #include "_ROOT_FOLDER_/res/shader-lib/kernel_convolution.glsl"

INPUT float Spread;

INPUT float Mask;

// https://en.wikipedia.org/wiki/Kernel_(image_processing)

#define sharpen float[9]( \
    0, -1, 0,             \
    -1, 5, -1,            \
    0, -1, 0              \
)

void main()
{
    vec2 in_uv = normalized_uv();
    CONVOLUTION(sharpen, sharpen.length(), image, in_uv, Spread, Mask);
    out_Color = vec4(color, 1.);
}