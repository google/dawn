#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4ccf9a() {
  uvec4 res = imageLoad(arg_0, ivec3(uvec3(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_4ccf9a();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

layout(binding = 0, rg32ui) uniform highp uimage3D arg_0;
uvec4 textureLoad_4ccf9a() {
  uvec4 res = imageLoad(arg_0, ivec3(uvec3(1u)));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_4ccf9a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
