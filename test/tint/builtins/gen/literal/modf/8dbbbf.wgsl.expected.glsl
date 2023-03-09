#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  modf_result_f16 res = modf_result_f16(-0.5hf, -1.0hf);
}

vec4 vertex_main() {
  modf_8dbbbf();
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

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  modf_result_f16 res = modf_result_f16(-0.5hf, -1.0hf);
}

void fragment_main() {
  modf_8dbbbf();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  modf_result_f16 res = modf_result_f16(-0.5hf, -1.0hf);
}

void compute_main() {
  modf_8dbbbf();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
