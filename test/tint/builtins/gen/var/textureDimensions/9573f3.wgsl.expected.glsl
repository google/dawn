//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
layout(binding = 1, rg32ui) uniform highp writeonly uimage2D f_arg_0;
uvec2 textureDimensions_9573f3() {
  uvec2 res = uvec2(imageSize(f_arg_0));
  return res;
}
void main() {
  v.inner = textureDimensions_9573f3();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 1, rg32ui) uniform highp writeonly uimage2D arg_0;
uvec2 textureDimensions_9573f3() {
  uvec2 res = uvec2(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_9573f3();
}
