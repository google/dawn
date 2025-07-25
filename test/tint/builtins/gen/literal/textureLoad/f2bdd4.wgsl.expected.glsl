//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 1, r8) uniform highp image2DArray f_arg_0;
vec4 textureLoad_f2bdd4() {
  uint v_1 = min(1u, (uint(imageSize(f_arg_0).z) - 1u));
  ivec2 v_2 = ivec2(min(uvec2(1u), (uvec2(imageSize(f_arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(f_arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_f2bdd4();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 1, r8) uniform highp image2DArray arg_0;
vec4 textureLoad_f2bdd4() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_2 = ivec2(min(uvec2(1u), (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_f2bdd4();
}
