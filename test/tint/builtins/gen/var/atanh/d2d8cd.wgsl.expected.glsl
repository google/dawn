#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_atanh(float16_t x) {
  return ((x >= 1.0hf) ? 0.0hf : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atanh_d2d8cd();
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

float16_t tint_atanh(float16_t x) {
  return ((x >= 1.0hf) ? 0.0hf : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  atanh_d2d8cd();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_atanh(float16_t x) {
  return ((x >= 1.0hf) ? 0.0hf : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void atanh_d2d8cd() {
  float16_t arg_0 = 0.5hf;
  float16_t res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  atanh_d2d8cd();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
