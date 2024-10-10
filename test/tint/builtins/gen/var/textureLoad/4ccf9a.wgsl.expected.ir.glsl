#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4ccf9a() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}
void main() {
  v.inner = textureLoad_4ccf9a();
}
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4ccf9a() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4ccf9a();
}
