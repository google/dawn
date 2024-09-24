#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_4 = ivec4((uvec4(v_1) << v_3));
  ivec4 v_5 = (v_4 >> uvec4(24u));
  uvec4 v_6 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_7 = ivec4((uvec4(v_2) << v_6));
  int res = tint_int_dot(v_5, (v_7 >> uvec4(24u)));
  return res;
}
void main() {
  v.tint_symbol = dot4I8Packed_881e62();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_4 = ivec4((uvec4(v_1) << v_3));
  ivec4 v_5 = (v_4 >> uvec4(24u));
  uvec4 v_6 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_7 = ivec4((uvec4(v_2) << v_6));
  int res = tint_int_dot(v_5, (v_7 >> uvec4(24u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = dot4I8Packed_881e62();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_3 = ivec4((uvec4(v) << v_2));
  ivec4 v_4 = (v_3 >> uvec4(24u));
  uvec4 v_5 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_6 = ivec4((uvec4(v_1) << v_5));
  int res = tint_int_dot(v_4, (v_6 >> uvec4(24u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4I8Packed_881e62();
  return tint_symbol;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = v_7.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
