#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uvec4 countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v_1 = arg_0;
  uvec4 v_2 = mix(uvec4(0u), uvec4(16u), equal((v_1 & uvec4(65535u)), uvec4(0u)));
  uvec4 v_3 = mix(uvec4(0u), uvec4(8u), equal(((v_1 >> v_2) & uvec4(255u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(0u), uvec4(4u), equal((((v_1 >> v_2) >> v_3) & uvec4(15u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(0u), uvec4(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec4(3u)), uvec4(0u)));
  uvec4 v_6 = mix(uvec4(0u), uvec4(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec4(1u)), uvec4(0u)));
  uvec4 res = ((v_2 | (v_3 | (v_4 | (v_5 | v_6)))) + mix(uvec4(0u), uvec4(1u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec4(0u))));
  return res;
}
void main() {
  v.inner = countTrailingZeros_d2b4a0();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uvec4 countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v_1 = arg_0;
  uvec4 v_2 = mix(uvec4(0u), uvec4(16u), equal((v_1 & uvec4(65535u)), uvec4(0u)));
  uvec4 v_3 = mix(uvec4(0u), uvec4(8u), equal(((v_1 >> v_2) & uvec4(255u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(0u), uvec4(4u), equal((((v_1 >> v_2) >> v_3) & uvec4(15u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(0u), uvec4(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec4(3u)), uvec4(0u)));
  uvec4 v_6 = mix(uvec4(0u), uvec4(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec4(1u)), uvec4(0u)));
  uvec4 res = ((v_2 | (v_3 | (v_4 | (v_5 | v_6)))) + mix(uvec4(0u), uvec4(1u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec4(0u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countTrailingZeros_d2b4a0();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v = arg_0;
  uvec4 v_1 = mix(uvec4(0u), uvec4(16u), equal((v & uvec4(65535u)), uvec4(0u)));
  uvec4 v_2 = mix(uvec4(0u), uvec4(8u), equal(((v >> v_1) & uvec4(255u)), uvec4(0u)));
  uvec4 v_3 = mix(uvec4(0u), uvec4(4u), equal((((v >> v_1) >> v_2) & uvec4(15u)), uvec4(0u)));
  uvec4 v_4 = mix(uvec4(0u), uvec4(2u), equal(((((v >> v_1) >> v_2) >> v_3) & uvec4(3u)), uvec4(0u)));
  uvec4 v_5 = mix(uvec4(0u), uvec4(1u), equal((((((v >> v_1) >> v_2) >> v_3) >> v_4) & uvec4(1u)), uvec4(0u)));
  uvec4 res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + mix(uvec4(0u), uvec4(1u), equal(((((v >> v_1) >> v_2) >> v_3) >> v_4), uvec4(0u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_d2b4a0();
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
