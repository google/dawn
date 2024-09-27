#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_c41bd1() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bool arg_2 = true;
  bvec4 v_1 = arg_0;
  bvec4 v_2 = arg_1;
  bvec4 res = mix(v_1, v_2, bvec4(arg_2));
  return mix(0, 1, all(equal(res, bvec4(false))));
}
void main() {
  v.tint_symbol = select_c41bd1();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int select_c41bd1() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bool arg_2 = true;
  bvec4 v_1 = arg_0;
  bvec4 v_2 = arg_1;
  bvec4 res = mix(v_1, v_2, bvec4(arg_2));
  return mix(0, 1, all(equal(res, bvec4(false))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_c41bd1();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int select_c41bd1() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bool arg_2 = true;
  bvec4 v = arg_0;
  bvec4 v_1 = arg_1;
  bvec4 res = mix(v, v_1, bvec4(arg_2));
  return mix(0, 1, all(equal(res, bvec4(false))));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_c41bd1();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
