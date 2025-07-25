//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
layout(binding = 1, r32i) uniform highp iimage2D f_arg_0;
ivec4 textureLoad_c80691() {
  uint arg_1 = 1u;
  uint v_1 = arg_1;
  ivec4 res = imageLoad(f_arg_0, ivec2(uvec2(min(v_1, (uvec2(imageSize(f_arg_0)).x - 1u)), 0u)));
  return res;
}
void main() {
  v.inner = textureLoad_c80691();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 1, r32i) uniform highp iimage2D arg_0;
ivec4 textureLoad_c80691() {
  uint arg_1 = 1u;
  uint v_1 = arg_1;
  ivec4 res = imageLoad(arg_0, ivec2(uvec2(min(v_1, (uvec2(imageSize(arg_0)).x - 1u)), 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c80691();
}
