#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp sampler3D arg_0_arg_1;
vec4 textureSample_2149ec() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(1.0f), ivec3(1));
  return res;
}
void main() {
  v.inner = textureSample_2149ec();
}
