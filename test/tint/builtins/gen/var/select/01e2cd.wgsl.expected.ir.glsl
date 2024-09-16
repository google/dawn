#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 v_1 = arg_0;
  ivec3 v_2 = arg_1;
  bvec3 v_3 = arg_2;
  int v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  int v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  ivec3 res = ivec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return res;
}
void main() {
  v.tint_symbol = select_01e2cd();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 v_1 = arg_0;
  ivec3 v_2 = arg_1;
  bvec3 v_3 = arg_2;
  int v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  int v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  ivec3 res = ivec3(v_4, v_5, ((v_3.z) ? (v_2.z) : (v_1.z)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_01e2cd();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 v = arg_0;
  ivec3 v_1 = arg_1;
  bvec3 v_2 = arg_2;
  int v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  int v_4 = ((v_2.y) ? (v_1.y) : (v.y));
  ivec3 res = ivec3(v_3, v_4, ((v_2.z) ? (v_1.z) : (v.z)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_01e2cd();
  return tint_symbol;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = v_5.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
