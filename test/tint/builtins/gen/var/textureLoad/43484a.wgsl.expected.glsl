#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, r32f) uniform highp image2DArray arg_0;
vec4 textureLoad_43484a() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 res = imageLoad(arg_0, ivec3(uvec3(arg_1, arg_2)));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureLoad_43484a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

layout(binding = 0, r32f) uniform highp image2DArray arg_0;
vec4 textureLoad_43484a() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  vec4 res = imageLoad(arg_0, ivec3(uvec3(arg_1, arg_2)));
  return res;
}

void compute_main() {
  prevent_dce.inner = textureLoad_43484a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
