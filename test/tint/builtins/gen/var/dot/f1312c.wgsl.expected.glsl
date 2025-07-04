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
int tint_int_dot(ivec3 x, ivec3 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  int v_6 = int((v_5 + uint(v_4)));
  uint v_7 = uint(x.z);
  int v_8 = int((v_7 * uint(y.z)));
  uint v_9 = uint(v_6);
  return int((v_9 + uint(v_8)));
}
int dot_f1312c() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
void main() {
  v.inner = dot_f1312c();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int tint_int_dot(ivec3 x, ivec3 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  int v_6 = int((v_5 + uint(v_4)));
  uint v_7 = uint(x.z);
  int v_8 = int((v_7 * uint(y.z)));
  uint v_9 = uint(v_6);
  return int((v_9 + uint(v_8)));
}
int dot_f1312c() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = dot_f1312c();
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
int tint_int_dot(ivec3 x, ivec3 y) {
  uint v = uint(x.x);
  int v_1 = int((v * uint(y.x)));
  uint v_2 = uint(x.y);
  int v_3 = int((v_2 * uint(y.y)));
  uint v_4 = uint(v_1);
  int v_5 = int((v_4 + uint(v_3)));
  uint v_6 = uint(x.z);
  int v_7 = int((v_6 * uint(y.z)));
  uint v_8 = uint(v_5);
  return int((v_8 + uint(v_7)));
}
int dot_f1312c() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  int res = tint_int_dot(arg_0, arg_1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_9 = VertexOutput(vec4(0.0f), 0);
  v_9.pos = vec4(0.0f);
  v_9.prevent_dce = dot_f1312c();
  return v_9;
}
void main() {
  VertexOutput v_10 = vertex_main_inner();
  gl_Position = vec4(v_10.pos.x, -(v_10.pos.y), ((2.0f * v_10.pos.z) - v_10.pos.w), v_10.pos.w);
  tint_interstage_location0 = v_10.prevent_dce;
  gl_PointSize = 1.0f;
}
