#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_radians(float16_t param_0) {
  return param_0 * 0.01745329251994329547hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void radians_208fd9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_radians(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  radians_208fd9();
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

float16_t tint_radians(float16_t param_0) {
  return param_0 * 0.01745329251994329547hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void radians_208fd9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_radians(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  radians_208fd9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_radians(float16_t param_0) {
  return param_0 * 0.01745329251994329547hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void radians_208fd9() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_radians(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  radians_208fd9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
