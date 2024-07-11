#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba32ui) uniform highp writeonly uimage2D arg_0;
uint textureDimensions_09140b() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureDimensions_09140b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba32ui) uniform highp writeonly uimage2D arg_0;
uint textureDimensions_09140b() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}

void compute_main() {
  prevent_dce.inner = textureDimensions_09140b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
