//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
layout(binding = 1, rg32ui) uniform highp uimage2DArray f_arg_0;
uvec4 textureLoad_34d97c() {
  uint v_1 = (uint(imageSize(f_arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(f_arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  uvec4 res = imageLoad(f_arg_0, ivec3(v_4, int(v_2)));
  return res;
}
void main() {
  v.inner = textureLoad_34d97c();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 1, rg32ui) uniform highp uimage2DArray arg_0;
uvec4 textureLoad_34d97c() {
  uint v_1 = (uint(imageSize(arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  uvec4 res = imageLoad(arg_0, ivec3(v_4, int(v_2)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_34d97c();
}
