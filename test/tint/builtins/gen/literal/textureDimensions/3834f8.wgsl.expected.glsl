//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
layout(binding = 1, r32i) uniform highp iimage2DArray f_arg_0;
uvec2 textureDimensions_3834f8() {
  uvec2 res = uvec2(imageSize(f_arg_0).xy);
  return res;
}
void main() {
  v.inner = textureDimensions_3834f8();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 1, r32i) uniform highp iimage2DArray arg_0;
uvec2 textureDimensions_3834f8() {
  uvec2 res = uvec2(imageSize(arg_0).xy);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_3834f8();
}
