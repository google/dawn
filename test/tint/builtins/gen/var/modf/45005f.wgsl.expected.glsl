#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

struct modf_result_vec3_f16 {
  f16vec3 fract;
  f16vec3 whole;
};

modf_result_vec3_f16 tint_modf(f16vec3 param_0) {
  modf_result_vec3_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_45005f() {
  f16vec3 arg_0 = f16vec3(-1.5hf);
  modf_result_vec3_f16 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

void fragment_main() {
  modf_45005f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec3_f16 {
  f16vec3 fract;
  f16vec3 whole;
};

modf_result_vec3_f16 tint_modf(f16vec3 param_0) {
  modf_result_vec3_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_45005f() {
  f16vec3 arg_0 = f16vec3(-1.5hf);
  modf_result_vec3_f16 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  modf_45005f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec3_f16 {
  f16vec3 fract;
  f16vec3 whole;
};

modf_result_vec3_f16 tint_modf(f16vec3 param_0) {
  modf_result_vec3_f16 result;
  result.fract = modf(param_0, result.whole);
  return result;
}


void modf_45005f() {
  f16vec3 arg_0 = f16vec3(-1.5hf);
  modf_result_vec3_f16 res = tint_modf(arg_0);
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_45005f();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
