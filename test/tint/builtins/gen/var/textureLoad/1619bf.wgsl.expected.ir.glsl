#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, r32i) uniform highp iimage2DArray arg_0;
ivec4 textureLoad_1619bf() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  ivec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_1619bf();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, r32i) uniform highp iimage2DArray arg_0;
ivec4 textureLoad_1619bf() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  ivec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_1619bf();
}
