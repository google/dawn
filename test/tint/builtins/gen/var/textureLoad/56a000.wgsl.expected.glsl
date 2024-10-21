#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rg32f) uniform highp image2D arg_0;
vec4 textureLoad_56a000() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(ivec2(arg_1, 0)));
  return res;
}
void main() {
  v.inner = textureLoad_56a000();
}
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rg32f) uniform highp image2D arg_0;
vec4 textureLoad_56a000() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(ivec2(arg_1, 0)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_56a000();
}
