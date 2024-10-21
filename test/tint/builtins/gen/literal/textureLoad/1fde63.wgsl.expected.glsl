#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image3D arg_0;
vec4 textureLoad_1fde63() {
  vec4 res = imageLoad(arg_0, ivec3(uvec3(1u)));
  return res;
}
void main() {
  v.inner = textureLoad_1fde63();
}
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image3D arg_0;
vec4 textureLoad_1fde63() {
  vec4 res = imageLoad(arg_0, ivec3(uvec3(1u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_1fde63();
}
