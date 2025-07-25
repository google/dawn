//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  int inner;
} v;
int tint_int_dot(ivec2 x, ivec2 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  return int((v_5 + uint(v_4)));
}
int dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
void main() {
  v.inner = dot_fc5f7c();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int tint_int_dot(ivec2 x, ivec2 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  return int((v_5 + uint(v_4)));
}
int dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = dot_fc5f7c();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int tint_interstage_location0;
int tint_int_dot(ivec2 x, ivec2 y) {
  uint v = uint(x.x);
  int v_1 = int((v * uint(y.x)));
  uint v_2 = uint(x.y);
  int v_3 = int((v_2 * uint(y.y)));
  uint v_4 = uint(v_1);
  return int((v_4 + uint(v_3)));
}
int dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), 0);
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = dot_fc5f7c();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
