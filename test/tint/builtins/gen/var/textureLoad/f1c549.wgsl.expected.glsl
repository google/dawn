#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, r32f) uniform highp image3D arg_0;
vec4 textureLoad_f1c549() {
  ivec3 arg_1 = ivec3(1);
  vec4 res = imageLoad(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_f1c549();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, r32f) uniform highp image3D arg_0;
vec4 textureLoad_f1c549() {
  ivec3 arg_1 = ivec3(1);
  vec4 res = imageLoad(arg_0, arg_1);
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_f1c549();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
