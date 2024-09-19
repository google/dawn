#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image3D arg_0;
uvec3 textureDimensions_ae4595() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_ae4595();
}

void main() {
  fragment_main();
  return;
}
#version 460

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec3 inner;
} prevent_dce;

layout(binding = 0, rg32f) uniform highp image3D arg_0;
uvec3 textureDimensions_ae4595() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_ae4595();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
