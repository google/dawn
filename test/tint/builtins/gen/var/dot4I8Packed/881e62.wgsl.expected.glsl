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
int tint_int_dot(ivec4 x, ivec4 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  int v_6 = int((v_5 + uint(v_4)));
  uint v_7 = uint(x.z);
  int v_8 = int((v_7 * uint(y.z)));
  uint v_9 = uint(v_6);
  int v_10 = int((v_9 + uint(v_8)));
  uint v_11 = uint(x.w);
  int v_12 = int((v_11 * uint(y.w)));
  uint v_13 = uint(v_10);
  return int((v_13 + uint(v_12)));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_14 = arg_0;
  uint v_15 = arg_1;
  uvec4 v_16 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_17 = ivec4((uvec4(v_14) << v_16));
  ivec4 v_18 = (v_17 >> uvec4(24u));
  uvec4 v_19 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_20 = ivec4((uvec4(v_15) << v_19));
  int res = tint_int_dot(v_18, (v_20 >> uvec4(24u)));
  return res;
}
void main() {
  v.inner = dot4I8Packed_881e62();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int tint_int_dot(ivec4 x, ivec4 y) {
  uint v_1 = uint(x.x);
  int v_2 = int((v_1 * uint(y.x)));
  uint v_3 = uint(x.y);
  int v_4 = int((v_3 * uint(y.y)));
  uint v_5 = uint(v_2);
  int v_6 = int((v_5 + uint(v_4)));
  uint v_7 = uint(x.z);
  int v_8 = int((v_7 * uint(y.z)));
  uint v_9 = uint(v_6);
  int v_10 = int((v_9 + uint(v_8)));
  uint v_11 = uint(x.w);
  int v_12 = int((v_11 * uint(y.w)));
  uint v_13 = uint(v_10);
  return int((v_13 + uint(v_12)));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_14 = arg_0;
  uint v_15 = arg_1;
  uvec4 v_16 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_17 = ivec4((uvec4(v_14) << v_16));
  ivec4 v_18 = (v_17 >> uvec4(24u));
  uvec4 v_19 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_20 = ivec4((uvec4(v_15) << v_19));
  int res = tint_int_dot(v_18, (v_20 >> uvec4(24u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = dot4I8Packed_881e62();
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
int tint_int_dot(ivec4 x, ivec4 y) {
  uint v = uint(x.x);
  int v_1 = int((v * uint(y.x)));
  uint v_2 = uint(x.y);
  int v_3 = int((v_2 * uint(y.y)));
  uint v_4 = uint(v_1);
  int v_5 = int((v_4 + uint(v_3)));
  uint v_6 = uint(x.z);
  int v_7 = int((v_6 * uint(y.z)));
  uint v_8 = uint(v_5);
  int v_9 = int((v_8 + uint(v_7)));
  uint v_10 = uint(x.w);
  int v_11 = int((v_10 * uint(y.w)));
  uint v_12 = uint(v_9);
  return int((v_12 + uint(v_11)));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_13 = arg_0;
  uint v_14 = arg_1;
  uvec4 v_15 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_16 = ivec4((uvec4(v_13) << v_15));
  ivec4 v_17 = (v_16 >> uvec4(24u));
  uvec4 v_18 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_19 = ivec4((uvec4(v_14) << v_18));
  int res = tint_int_dot(v_17, (v_19 >> uvec4(24u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_20 = VertexOutput(vec4(0.0f), 0);
  v_20.pos = vec4(0.0f);
  v_20.prevent_dce = dot4I8Packed_881e62();
  return v_20;
}
void main() {
  VertexOutput v_21 = vertex_main_inner();
  gl_Position = vec4(v_21.pos.x, -(v_21.pos.y), ((2.0f * v_21.pos.z) - v_21.pos.w), v_21.pos.w);
  tint_interstage_location0 = v_21.prevent_dce;
  gl_PointSize = 1.0f;
}
