#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
uint textureDimensions_84f363() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_84f363();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba8) uniform highp writeonly image2D arg_0;
uint textureDimensions_84f363() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_84f363();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
