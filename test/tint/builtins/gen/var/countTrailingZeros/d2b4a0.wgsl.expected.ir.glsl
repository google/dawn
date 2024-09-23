#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
uvec4 countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v_1 = arg_0;
  bvec4 v_2 = equal((v_1 & uvec4(65535u)), uvec4(0u));
  uint v_3 = ((v_2.x) ? (uvec4(16u).x) : (uvec4(0u).x));
  uint v_4 = ((v_2.y) ? (uvec4(16u).y) : (uvec4(0u).y));
  uint v_5 = ((v_2.z) ? (uvec4(16u).z) : (uvec4(0u).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (uvec4(16u).w) : (uvec4(0u).w)));
  bvec4 v_7 = equal(((v_1 >> v_6) & uvec4(255u)), uvec4(0u));
  uint v_8 = ((v_7.x) ? (uvec4(8u).x) : (uvec4(0u).x));
  uint v_9 = ((v_7.y) ? (uvec4(8u).y) : (uvec4(0u).y));
  uint v_10 = ((v_7.z) ? (uvec4(8u).z) : (uvec4(0u).z));
  uvec4 v_11 = uvec4(v_8, v_9, v_10, ((v_7.w) ? (uvec4(8u).w) : (uvec4(0u).w)));
  bvec4 v_12 = equal((((v_1 >> v_6) >> v_11) & uvec4(15u)), uvec4(0u));
  uint v_13 = ((v_12.x) ? (uvec4(4u).x) : (uvec4(0u).x));
  uint v_14 = ((v_12.y) ? (uvec4(4u).y) : (uvec4(0u).y));
  uint v_15 = ((v_12.z) ? (uvec4(4u).z) : (uvec4(0u).z));
  uvec4 v_16 = uvec4(v_13, v_14, v_15, ((v_12.w) ? (uvec4(4u).w) : (uvec4(0u).w)));
  bvec4 v_17 = equal(((((v_1 >> v_6) >> v_11) >> v_16) & uvec4(3u)), uvec4(0u));
  uint v_18 = ((v_17.x) ? (uvec4(2u).x) : (uvec4(0u).x));
  uint v_19 = ((v_17.y) ? (uvec4(2u).y) : (uvec4(0u).y));
  uint v_20 = ((v_17.z) ? (uvec4(2u).z) : (uvec4(0u).z));
  uvec4 v_21 = uvec4(v_18, v_19, v_20, ((v_17.w) ? (uvec4(2u).w) : (uvec4(0u).w)));
  bvec4 v_22 = equal((((((v_1 >> v_6) >> v_11) >> v_16) >> v_21) & uvec4(1u)), uvec4(0u));
  uint v_23 = ((v_22.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_24 = ((v_22.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_25 = ((v_22.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 v_26 = uvec4(v_23, v_24, v_25, ((v_22.w) ? (uvec4(1u).w) : (uvec4(0u).w)));
  bvec4 v_27 = equal(((((v_1 >> v_6) >> v_11) >> v_16) >> v_21), uvec4(0u));
  uint v_28 = ((v_27.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_29 = ((v_27.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_30 = ((v_27.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 res = ((v_6 | (v_11 | (v_16 | (v_21 | v_26)))) + uvec4(v_28, v_29, v_30, ((v_27.w) ? (uvec4(1u).w) : (uvec4(0u).w))));
  return res;
}
void main() {
  v.tint_symbol = countTrailingZeros_d2b4a0();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
uvec4 countTrailingZeros_d2b4a0() {
  uvec4 arg_0 = uvec4(1u);
  uvec4 v_1 = arg_0;
  bvec4 v_2 = equal((v_1 & uvec4(65535u)), uvec4(0u));
  uint v_3 = ((v_2.x) ? (uvec4(16u).x) : (uvec4(0u).x));
  uint v_4 = ((v_2.y) ? (uvec4(16u).y) : (uvec4(0u).y));
  uint v_5 = ((v_2.z) ? (uvec4(16u).z) : (uvec4(0u).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (uvec4(16u).w) : (uvec4(0u).w)));
  bvec4 v_7 = equal(((v_1 >> v_6) & uvec4(255u)), uvec4(0u));
  uint v_8 = ((v_7.x) ? (uvec4(8u).x) : (uvec4(0u).x));
  uint v_9 = ((v_7.y) ? (uvec4(8u).y) : (uvec4(0u).y));
  uint v_10 = ((v_7.z) ? (uvec4(8u).z) : (uvec4(0u).z));
  uvec4 v_11 = uvec4(v_8, v_9, v_10, ((v_7.w) ? (uvec4(8u).w) : (uvec4(0u).w)));
  bvec4 v_12 = equal((((v_1 >> v_6) >> v_11) & uvec4(15u)), uvec4(0u));
  uint v_13 = ((v_12.x) ? (uvec4(4u).x) : (uvec4(0u).x));
  uint v_14 = ((v_12.y) ? (uvec4(4u).y) : (uvec4(0u).y));
  uint v_15 = ((v_12.z) ? (uvec4(4u).z) : (uvec4(0u).z));
  uvec4 v_16 = uvec4(v_13, v_14, v_15, ((v_12.w) ? (uvec4(4u).w) : (uvec4(0u).w)));
  bvec4 v_17 = equal(((((v_1 >> v_6) >> v_11) >> v_16) & uvec4(3u)), uvec4(0u));
  uint v_18 = ((v_17.x) ? (uvec4(2u).x) : (uvec4(0u).x));
  uint v_19 = ((v_17.y) ? (uvec4(2u).y) : (uvec4(0u).y));
  uint v_20 = ((v_17.z) ? (uvec4(2u).z) : (uvec4(0u).z));
  uvec4 v_21 = uvec4(v_18, v_19, v_20, ((v_17.w) ? (uvec4(2u).w) : (uvec4(0u).w)));
  bvec4 v_22 = equal((((((v_1 >> v_6) >> v_11) >> v_16) >> v_21) & uvec4(1u)), uvec4(0u));
  uint v_23 = ((v_22.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_24 = ((v_22.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_25 = ((v_22.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 v_26 = uvec4(v_23, v_24, v_25, ((v_22.w) ? (uvec4(1u).w) : (uvec4(0u).w)));
  bvec4 v_27 = equal(((((v_1 >> v_6) >> v_11) >> v_16) >> v_21), uvec4(0u));
  uint v_28 = ((v_27.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_29 = ((v_27.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_30 = ((v_27.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 res = ((v_6 | (v_11 | (v_16 | (v_21 | v_26)))) + uvec4(v_28, v_29, v_30, ((v_27.w) ? (uvec4(1u).w) : (uvec4(0u).w))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countTrailingZeros_d2b4a0();
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
  bvec4 v_1 = equal((v & uvec4(65535u)), uvec4(0u));
  uint v_2 = ((v_1.x) ? (uvec4(16u).x) : (uvec4(0u).x));
  uint v_3 = ((v_1.y) ? (uvec4(16u).y) : (uvec4(0u).y));
  uint v_4 = ((v_1.z) ? (uvec4(16u).z) : (uvec4(0u).z));
  uvec4 v_5 = uvec4(v_2, v_3, v_4, ((v_1.w) ? (uvec4(16u).w) : (uvec4(0u).w)));
  bvec4 v_6 = equal(((v >> v_5) & uvec4(255u)), uvec4(0u));
  uint v_7 = ((v_6.x) ? (uvec4(8u).x) : (uvec4(0u).x));
  uint v_8 = ((v_6.y) ? (uvec4(8u).y) : (uvec4(0u).y));
  uint v_9 = ((v_6.z) ? (uvec4(8u).z) : (uvec4(0u).z));
  uvec4 v_10 = uvec4(v_7, v_8, v_9, ((v_6.w) ? (uvec4(8u).w) : (uvec4(0u).w)));
  bvec4 v_11 = equal((((v >> v_5) >> v_10) & uvec4(15u)), uvec4(0u));
  uint v_12 = ((v_11.x) ? (uvec4(4u).x) : (uvec4(0u).x));
  uint v_13 = ((v_11.y) ? (uvec4(4u).y) : (uvec4(0u).y));
  uint v_14 = ((v_11.z) ? (uvec4(4u).z) : (uvec4(0u).z));
  uvec4 v_15 = uvec4(v_12, v_13, v_14, ((v_11.w) ? (uvec4(4u).w) : (uvec4(0u).w)));
  bvec4 v_16 = equal(((((v >> v_5) >> v_10) >> v_15) & uvec4(3u)), uvec4(0u));
  uint v_17 = ((v_16.x) ? (uvec4(2u).x) : (uvec4(0u).x));
  uint v_18 = ((v_16.y) ? (uvec4(2u).y) : (uvec4(0u).y));
  uint v_19 = ((v_16.z) ? (uvec4(2u).z) : (uvec4(0u).z));
  uvec4 v_20 = uvec4(v_17, v_18, v_19, ((v_16.w) ? (uvec4(2u).w) : (uvec4(0u).w)));
  bvec4 v_21 = equal((((((v >> v_5) >> v_10) >> v_15) >> v_20) & uvec4(1u)), uvec4(0u));
  uint v_22 = ((v_21.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_23 = ((v_21.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_24 = ((v_21.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 v_25 = uvec4(v_22, v_23, v_24, ((v_21.w) ? (uvec4(1u).w) : (uvec4(0u).w)));
  bvec4 v_26 = equal(((((v >> v_5) >> v_10) >> v_15) >> v_20), uvec4(0u));
  uint v_27 = ((v_26.x) ? (uvec4(1u).x) : (uvec4(0u).x));
  uint v_28 = ((v_26.y) ? (uvec4(1u).y) : (uvec4(0u).y));
  uint v_29 = ((v_26.z) ? (uvec4(1u).z) : (uvec4(0u).z));
  uvec4 res = ((v_5 | (v_10 | (v_15 | (v_20 | v_25)))) + uvec4(v_27, v_28, v_29, ((v_26.w) ? (uvec4(1u).w) : (uvec4(0u).w))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_d2b4a0();
  return tint_symbol;
}
void main() {
  VertexOutput v_30 = vertex_main_inner();
  gl_Position = v_30.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_30.prevent_dce;
  gl_PointSize = 1.0f;
}
