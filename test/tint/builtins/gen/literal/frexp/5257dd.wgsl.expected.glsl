//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct frexp_result_f16 {
  float16_t member_0;
  int member_1;
};

void frexp_5257dd() {
  frexp_result_f16 res = frexp_result_f16(0.5hf, 1);
}
void main() {
  frexp_5257dd();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_f16 {
  float16_t member_0;
  int member_1;
};

void frexp_5257dd() {
  frexp_result_f16 res = frexp_result_f16(0.5hf, 1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5257dd();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_f16 {
  float16_t member_0;
  int member_1;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_5257dd() {
  frexp_result_f16 res = frexp_result_f16(0.5hf, 1);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  frexp_5257dd();
  return v;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
