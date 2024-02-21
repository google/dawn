#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec2 tint_bitcast_to_f16(int src) {
  uint r = uint(src);
  return unpackFloat2x16(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void bitcast_674557() {
  int arg_0 = 1;
  f16vec2 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  bitcast_674557();
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
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(int src) {
  uint r = uint(src);
  return unpackFloat2x16(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void bitcast_674557() {
  int arg_0 = 1;
  f16vec2 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  bitcast_674557();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec2 tint_bitcast_to_f16(int src) {
  uint r = uint(src);
  return unpackFloat2x16(r);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec2 inner;
} prevent_dce;

void bitcast_674557() {
  int arg_0 = 1;
  f16vec2 res = tint_bitcast_to_f16(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  bitcast_674557();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
