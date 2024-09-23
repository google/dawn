#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 select_a2860e() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bvec4 arg_2 = bvec4(true);
  ivec4 v_1 = arg_0;
  ivec4 v_2 = arg_1;
  bvec4 v_3 = arg_2;
  int v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  int v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  int v_6 = ((v_3.z) ? (v_2.z) : (v_1.z));
  ivec4 res = ivec4(v_4, v_5, v_6, ((v_3.w) ? (v_2.w) : (v_1.w)));
  return res;
}
void main() {
  v.tint_symbol = select_a2860e();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 select_a2860e() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bvec4 arg_2 = bvec4(true);
  ivec4 v_1 = arg_0;
  ivec4 v_2 = arg_1;
  bvec4 v_3 = arg_2;
  int v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  int v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  int v_6 = ((v_3.z) ? (v_2.z) : (v_1.z));
  ivec4 res = ivec4(v_4, v_5, v_6, ((v_3.w) ? (v_2.w) : (v_1.w)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_a2860e();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 vertex_main_loc0_Output;
ivec4 select_a2860e() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  bvec4 arg_2 = bvec4(true);
  ivec4 v = arg_0;
  ivec4 v_1 = arg_1;
  bvec4 v_2 = arg_2;
  int v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  int v_4 = ((v_2.y) ? (v_1.y) : (v.y));
  int v_5 = ((v_2.z) ? (v_1.z) : (v.z));
  ivec4 res = ivec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (v.w)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec4(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_a2860e();
  return tint_symbol;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = v_6.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
