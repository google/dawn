#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
ivec4 textureLoad_b75c8f() {
  uvec2 arg_1 = uvec2(1u);
  ivec4 res = imageLoad(arg_0, ivec2(arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_b75c8f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

layout(binding = 0, r32i) uniform highp iimage2D arg_0;
ivec4 textureLoad_b75c8f() {
  uvec2 arg_1 = uvec2(1u);
  ivec4 res = imageLoad(arg_0, ivec2(arg_1));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_b75c8f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
