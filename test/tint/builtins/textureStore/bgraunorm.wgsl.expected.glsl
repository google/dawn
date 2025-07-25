#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, rgba8) uniform highp writeonly image2D f_tex;
void main() {
  vec4 value = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  imageStore(f_tex, ivec2(9, 8), value.zyxw);
}
