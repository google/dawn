#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v_1) >> v_2);
  uvec4 res = (v_3 & uvec4(255u));
  return res;
}
void main() {
  v.inner = unpack4xU8_a5ea55();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v_1) >> v_2);
  uvec4 res = (v_3 & uvec4(255u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = unpack4xU8_a5ea55();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = (uvec4(v) >> v_1);
  uvec4 res = (v_2 & uvec4(255u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = unpack4xU8_a5ea55();
  return tint_symbol;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = v_3.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
