//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
layout(binding = 1, rg32i) uniform highp iimage3D f_arg_0;
ivec4 textureLoad_d41c72() {
  uvec3 v_1 = (uvec3(imageSize(f_arg_0)) - uvec3(1u));
  ivec4 res = imageLoad(f_arg_0, ivec3(min(uvec3(ivec3(1)), v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_d41c72();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 1, rg32i) uniform highp iimage3D arg_0;
ivec4 textureLoad_d41c72() {
  uvec3 v_1 = (uvec3(imageSize(arg_0)) - uvec3(1u));
  ivec4 res = imageLoad(arg_0, ivec3(min(uvec3(ivec3(1)), v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_d41c72();
}
