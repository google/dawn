#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct modf_result_vec4_f16 {
  f16vec4 fract;
  f16vec4 whole;
};

void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
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
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
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
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
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
