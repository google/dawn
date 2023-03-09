#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

modf_result_vec4_f16 tint_modf(f16vec4 param_0) {
  modf_result_vec4_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 res = tint_modf(arg_0);
}

vec4 vertex_main() {
  modf_995934();
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

struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

modf_result_vec4_f16 tint_modf(f16vec4 param_0) {
  modf_result_vec4_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 res = tint_modf(arg_0);
}

void fragment_main() {
  modf_995934();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

modf_result_vec4_f16 tint_modf(f16vec4 param_0) {
  modf_result_vec4_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 res = tint_modf(arg_0);
}

void compute_main() {
  modf_995934();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
