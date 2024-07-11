#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

layout(binding = 0, r32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_19d6be() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_19d6be();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

layout(binding = 0, r32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_19d6be() {
  uvec3 arg_1 = uvec3(1u);
  uvec4 res = imageLoad(arg_0, ivec3(arg_1));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_19d6be();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
