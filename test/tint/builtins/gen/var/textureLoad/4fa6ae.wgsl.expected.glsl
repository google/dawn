#version 310 es

layout(rgba8) uniform highp readonly image3D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_4fa6ae() {
  uvec3 arg_1 = uvec3(1u);
  vec4 res = imageLoad(arg_0, ivec3(arg_1)).bgra;
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_4fa6ae();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(rgba8) uniform highp readonly image3D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_4fa6ae() {
  uvec3 arg_1 = uvec3(1u);
  vec4 res = imageLoad(arg_0, ivec3(arg_1)).bgra;
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_4fa6ae();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba8) uniform highp readonly image3D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_4fa6ae() {
  uvec3 arg_1 = uvec3(1u);
  vec4 res = imageLoad(arg_0, ivec3(arg_1)).bgra;
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_4fa6ae();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
