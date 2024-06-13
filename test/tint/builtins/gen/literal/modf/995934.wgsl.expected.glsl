#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};


void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}

struct VertexOutput {
  vec4 pos;
};

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


void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}

struct VertexOutput {
  vec4 pos;
};

void compute_main() {
  modf_995934();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};


void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}

struct VertexOutput {
  vec4 pos;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_995934();
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
