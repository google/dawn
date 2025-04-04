//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct frexp_result_vec4_f16 {
  f16vec4 member_0;
  ivec4 member_1;
};

void frexp_3dd21e() {
  frexp_result_vec4_f16 res = frexp_result_vec4_f16(f16vec4(0.5hf), ivec4(1));
}
void main() {
  frexp_3dd21e();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec4_f16 {
  f16vec4 member_0;
  ivec4 member_1;
};

void frexp_3dd21e() {
  frexp_result_vec4_f16 res = frexp_result_vec4_f16(f16vec4(0.5hf), ivec4(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_3dd21e();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec4_f16 {
  f16vec4 member_0;
  ivec4 member_1;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_3dd21e() {
  frexp_result_vec4_f16 res = frexp_result_vec4_f16(f16vec4(0.5hf), ivec4(1));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  frexp_3dd21e();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
