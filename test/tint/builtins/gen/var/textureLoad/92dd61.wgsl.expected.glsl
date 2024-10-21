#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image2D arg_0;
vec4 textureLoad_92dd61() {
  uint arg_1 = 1u;
  vec4 res = imageLoad(arg_0, ivec2(uvec2(arg_1, 0u)));
  return res;
}
void main() {
  v.inner = textureLoad_92dd61();
}
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image2D arg_0;
vec4 textureLoad_92dd61() {
  uint arg_1 = 1u;
  vec4 res = imageLoad(arg_0, ivec2(uvec2(arg_1, 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_92dd61();
}
