#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
uvec2 textureDimensions_c82420() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
void main() {
  v.tint_symbol = textureDimensions_c82420();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
layout(binding = 0, rg32ui) uniform highp uimage2D arg_0;
uvec2 textureDimensions_c82420() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureDimensions_c82420();
}
