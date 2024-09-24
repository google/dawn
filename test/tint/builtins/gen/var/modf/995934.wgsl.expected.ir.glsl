#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 v = modf_result_vec4_f16(f16vec4(0.0hf), f16vec4(0.0hf));
  v.fract = modf(arg_0, v.whole);
  modf_result_vec4_f16 res = v;
}
void main() {
  modf_995934();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 v = modf_result_vec4_f16(f16vec4(0.0hf), f16vec4(0.0hf));
  v.fract = modf(arg_0, v.whole);
  modf_result_vec4_f16 res = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_995934();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_995934() {
  f16vec4 arg_0 = f16vec4(-1.5hf);
  modf_result_vec4_f16 v = modf_result_vec4_f16(f16vec4(0.0hf), f16vec4(0.0hf));
  v.fract = modf(arg_0, v.whole);
  modf_result_vec4_f16 res = v;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_995934();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
