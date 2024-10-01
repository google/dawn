#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
layout(binding = 0, rgba16ui) uniform highp writeonly uimage2D arg_0;
uint textureDimensions_58a82d() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}
void main() {
  v.tint_symbol = textureDimensions_58a82d();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
layout(binding = 0, rgba16ui) uniform highp writeonly uimage2D arg_0;
uint textureDimensions_58a82d() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureDimensions_58a82d();
}
