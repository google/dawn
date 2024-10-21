#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 v = frexp_result_vec2_f16(f16vec2(0.0hf), ivec2(0));
  v.fract = frexp(arg_0, v.exp);
  frexp_result_vec2_f16 res = v;
}
void main() {
  frexp_5f47bf();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 v = frexp_result_vec2_f16(f16vec2(0.0hf), ivec2(0));
  v.fract = frexp(arg_0, v.exp);
  frexp_result_vec2_f16 res = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 v = frexp_result_vec2_f16(f16vec2(0.0hf), ivec2(0));
  v.fract = frexp(arg_0, v.exp);
  frexp_result_vec2_f16 res = v;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
