//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 1, rg32ui) uniform highp uimage2D f_arg_0;
uint textureDimensions_01e21e() {
  uint res = uvec2(imageSize(f_arg_0)).x;
  return res;
}
void main() {
  v.inner = textureDimensions_01e21e();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, rg32ui) uniform highp uimage2D arg_0;
uint textureDimensions_01e21e() {
  uint res = uvec2(imageSize(arg_0)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_01e21e();
}
