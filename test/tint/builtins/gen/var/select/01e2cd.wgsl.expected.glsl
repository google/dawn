//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec3 inner;
} v;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 res = mix(arg_0, arg_1, arg_2);
  return res;
}
void main() {
  v.inner = select_01e2cd();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec3 inner;
} v;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 res = mix(arg_0, arg_1, arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_01e2cd();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 tint_interstage_location0;
ivec3 select_01e2cd() {
  ivec3 arg_0 = ivec3(1);
  ivec3 arg_1 = ivec3(1);
  bvec3 arg_2 = bvec3(true);
  ivec3 res = mix(arg_0, arg_1, arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), ivec3(0));
  v.pos = vec4(0.0f);
  v.prevent_dce = select_01e2cd();
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
