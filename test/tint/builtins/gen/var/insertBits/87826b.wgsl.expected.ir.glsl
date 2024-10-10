#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 insertBits_87826b() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uvec3 v_1 = arg_0;
  uvec3 v_2 = arg_1;
  uint v_3 = arg_3;
  uint v_4 = min(arg_2, 32u);
  uint v_5 = min(v_3, (32u - v_4));
  int v_6 = int(v_4);
  uvec3 res = bitfieldInsert(v_1, v_2, v_6, int(v_5));
  return res;
}
void main() {
  v.inner = insertBits_87826b();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 insertBits_87826b() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uvec3 v_1 = arg_0;
  uvec3 v_2 = arg_1;
  uint v_3 = arg_3;
  uint v_4 = min(arg_2, 32u);
  uint v_5 = min(v_3, (32u - v_4));
  int v_6 = int(v_4);
  uvec3 res = bitfieldInsert(v_1, v_2, v_6, int(v_5));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = insertBits_87826b();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 insertBits_87826b() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 arg_1 = uvec3(1u);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uvec3 v = arg_0;
  uvec3 v_1 = arg_1;
  uint v_2 = arg_3;
  uint v_3 = min(arg_2, 32u);
  uint v_4 = min(v_2, (32u - v_3));
  int v_5 = int(v_3);
  uvec3 res = bitfieldInsert(v, v_1, v_5, int(v_4));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = insertBits_87826b();
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
