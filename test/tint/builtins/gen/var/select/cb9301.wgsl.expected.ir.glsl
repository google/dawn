#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 v_1 = arg_0;
  bvec2 v_2 = arg_1;
  bvec2 v_3 = arg_2;
  bool v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  bvec2 res = bvec2(v_4, ((v_3.y) ? (v_2.y) : (v_1.y)));
  return ((all(equal(res, bvec2(false)))) ? (1) : (0));
}
void main() {
  v.tint_symbol = select_cb9301();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 v_1 = arg_0;
  bvec2 v_2 = arg_1;
  bvec2 v_3 = arg_2;
  bool v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  bvec2 res = bvec2(v_4, ((v_3.y) ? (v_2.y) : (v_1.y)));
  return ((all(equal(res, bvec2(false)))) ? (1) : (0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_cb9301();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int select_cb9301() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bvec2 arg_2 = bvec2(true);
  bvec2 v = arg_0;
  bvec2 v_1 = arg_1;
  bvec2 v_2 = arg_2;
  bool v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  bvec2 res = bvec2(v_3, ((v_2.y) ? (v_1.y) : (v.y)));
  return ((all(equal(res, bvec2(false)))) ? (1) : (0));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_cb9301();
  return tint_symbol;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = v_4.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
