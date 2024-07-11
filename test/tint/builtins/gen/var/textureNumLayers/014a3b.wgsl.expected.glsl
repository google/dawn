#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
uint textureNumLayers_014a3b() {
  uint res = uint(imageSize(arg_0).z);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureNumLayers_014a3b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

layout(binding = 0, rgba16ui) uniform highp writeonly uimage2DArray arg_0;
uint textureNumLayers_014a3b() {
  uint res = uint(imageSize(arg_0).z);
  return res;
}

void compute_main() {
  prevent_dce.inner = textureNumLayers_014a3b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
