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
int select_e3e028() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bvec4 arg_2 = bvec4(true);
  bvec4 res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, all(equal(res, bvec4(false))));
}
void main() {
  v.inner = select_e3e028();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int select_e3e028() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bvec4 arg_2 = bvec4(true);
  bvec4 res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, all(equal(res, bvec4(false))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_e3e028();
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
int select_e3e028() {
  bvec4 arg_0 = bvec4(true);
  bvec4 arg_1 = bvec4(true);
  bvec4 arg_2 = bvec4(true);
  bvec4 res = mix(arg_0, arg_1, arg_2);
  return mix(0, 1, all(equal(res, bvec4(false))));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0);
  v.pos = vec4(0.0f);
  v.prevent_dce = select_e3e028();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
