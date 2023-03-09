#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_vec3_f16 {
  f16vec3 fract;
  ivec3 exp;
};

frexp_result_vec3_f16 tint_frexp(f16vec3 param_0) {
  frexp_result_vec3_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void frexp_ae4a66() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  frexp_result_vec3_f16 res = tint_frexp(arg_0);
}

vec4 vertex_main() {
  frexp_ae4a66();
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

struct frexp_result_vec3_f16 {
  f16vec3 fract;
  ivec3 exp;
};

frexp_result_vec3_f16 tint_frexp(f16vec3 param_0) {
  frexp_result_vec3_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void frexp_ae4a66() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  frexp_result_vec3_f16 res = tint_frexp(arg_0);
}

void fragment_main() {
  frexp_ae4a66();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct frexp_result_vec3_f16 {
  f16vec3 fract;
  ivec3 exp;
};

frexp_result_vec3_f16 tint_frexp(f16vec3 param_0) {
  frexp_result_vec3_f16 result;
  result.fract = frexp(param_0, result.exp);
  return result;
}


void frexp_ae4a66() {
  f16vec3 arg_0 = f16vec3(1.0hf);
  frexp_result_vec3_f16 res = tint_frexp(arg_0);
}

void compute_main() {
  frexp_ae4a66();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
