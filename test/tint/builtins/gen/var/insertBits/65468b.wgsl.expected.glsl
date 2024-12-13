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
int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int v_1 = arg_0;
  int v_2 = arg_1;
  uint v_3 = min(arg_2, 32u);
  uint v_4 = min(arg_3, (32u - v_3));
  int v_5 = int(v_3);
  int res = bitfieldInsert(v_1, v_2, v_5, int(v_4));
  return res;
}
void main() {
  v.inner = insertBits_65468b();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int v_1 = arg_0;
  int v_2 = arg_1;
  uint v_3 = min(arg_2, 32u);
  uint v_4 = min(arg_3, (32u - v_3));
  int v_5 = int(v_3);
  int res = bitfieldInsert(v_1, v_2, v_5, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = insertBits_65468b();
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
int insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int v = arg_0;
  int v_1 = arg_1;
  uint v_2 = min(arg_2, 32u);
  uint v_3 = min(arg_3, (32u - v_2));
  int v_4 = int(v_2);
  int res = bitfieldInsert(v, v_1, v_4, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = insertBits_65468b();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
