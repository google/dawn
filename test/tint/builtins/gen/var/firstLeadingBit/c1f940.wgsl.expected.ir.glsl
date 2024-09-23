#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  bvec4 v_2 = lessThan(v_1, uvec4(2147483648u));
  uint v_3 = ((v_2.x) ? (v_1.x) : (~(v_1).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (~(v_1).y));
  uint v_5 = ((v_2.z) ? (v_1.z) : (~(v_1).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (~(v_1).w)));
  bvec4 v_7 = equal((v_6 & uvec4(4294901760u)), uvec4(0u));
  uint v_8 = ((v_7.x) ? (uvec4(0u).x) : (uvec4(16u).x));
  uint v_9 = ((v_7.y) ? (uvec4(0u).y) : (uvec4(16u).y));
  uint v_10 = ((v_7.z) ? (uvec4(0u).z) : (uvec4(16u).z));
  uvec4 v_11 = uvec4(v_8, v_9, v_10, ((v_7.w) ? (uvec4(0u).w) : (uvec4(16u).w)));
  bvec4 v_12 = equal(((v_6 >> v_11) & uvec4(65280u)), uvec4(0u));
  uint v_13 = ((v_12.x) ? (uvec4(0u).x) : (uvec4(8u).x));
  uint v_14 = ((v_12.y) ? (uvec4(0u).y) : (uvec4(8u).y));
  uint v_15 = ((v_12.z) ? (uvec4(0u).z) : (uvec4(8u).z));
  uvec4 v_16 = uvec4(v_13, v_14, v_15, ((v_12.w) ? (uvec4(0u).w) : (uvec4(8u).w)));
  bvec4 v_17 = equal((((v_6 >> v_11) >> v_16) & uvec4(240u)), uvec4(0u));
  uint v_18 = ((v_17.x) ? (uvec4(0u).x) : (uvec4(4u).x));
  uint v_19 = ((v_17.y) ? (uvec4(0u).y) : (uvec4(4u).y));
  uint v_20 = ((v_17.z) ? (uvec4(0u).z) : (uvec4(4u).z));
  uvec4 v_21 = uvec4(v_18, v_19, v_20, ((v_17.w) ? (uvec4(0u).w) : (uvec4(4u).w)));
  bvec4 v_22 = equal(((((v_6 >> v_11) >> v_16) >> v_21) & uvec4(12u)), uvec4(0u));
  uint v_23 = ((v_22.x) ? (uvec4(0u).x) : (uvec4(2u).x));
  uint v_24 = ((v_22.y) ? (uvec4(0u).y) : (uvec4(2u).y));
  uint v_25 = ((v_22.z) ? (uvec4(0u).z) : (uvec4(2u).z));
  uvec4 v_26 = uvec4(v_23, v_24, v_25, ((v_22.w) ? (uvec4(0u).w) : (uvec4(2u).w)));
  bvec4 v_27 = equal((((((v_6 >> v_11) >> v_16) >> v_21) >> v_26) & uvec4(2u)), uvec4(0u));
  uint v_28 = ((v_27.x) ? (uvec4(0u).x) : (uvec4(1u).x));
  uint v_29 = ((v_27.y) ? (uvec4(0u).y) : (uvec4(1u).y));
  uint v_30 = ((v_27.z) ? (uvec4(0u).z) : (uvec4(1u).z));
  uvec4 v_31 = (v_11 | (v_16 | (v_21 | (v_26 | uvec4(v_28, v_29, v_30, ((v_27.w) ? (uvec4(0u).w) : (uvec4(1u).w)))))));
  bvec4 v_32 = equal(((((v_6 >> v_11) >> v_16) >> v_21) >> v_26), uvec4(0u));
  uint v_33 = ((v_32.x) ? (uvec4(4294967295u).x) : (v_31.x));
  uint v_34 = ((v_32.y) ? (uvec4(4294967295u).y) : (v_31.y));
  uint v_35 = ((v_32.z) ? (uvec4(4294967295u).z) : (v_31.z));
  ivec4 res = ivec4(uvec4(v_33, v_34, v_35, ((v_32.w) ? (uvec4(4294967295u).w) : (v_31.w))));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_c1f940();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v_1 = uvec4(arg_0);
  bvec4 v_2 = lessThan(v_1, uvec4(2147483648u));
  uint v_3 = ((v_2.x) ? (v_1.x) : (~(v_1).x));
  uint v_4 = ((v_2.y) ? (v_1.y) : (~(v_1).y));
  uint v_5 = ((v_2.z) ? (v_1.z) : (~(v_1).z));
  uvec4 v_6 = uvec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (~(v_1).w)));
  bvec4 v_7 = equal((v_6 & uvec4(4294901760u)), uvec4(0u));
  uint v_8 = ((v_7.x) ? (uvec4(0u).x) : (uvec4(16u).x));
  uint v_9 = ((v_7.y) ? (uvec4(0u).y) : (uvec4(16u).y));
  uint v_10 = ((v_7.z) ? (uvec4(0u).z) : (uvec4(16u).z));
  uvec4 v_11 = uvec4(v_8, v_9, v_10, ((v_7.w) ? (uvec4(0u).w) : (uvec4(16u).w)));
  bvec4 v_12 = equal(((v_6 >> v_11) & uvec4(65280u)), uvec4(0u));
  uint v_13 = ((v_12.x) ? (uvec4(0u).x) : (uvec4(8u).x));
  uint v_14 = ((v_12.y) ? (uvec4(0u).y) : (uvec4(8u).y));
  uint v_15 = ((v_12.z) ? (uvec4(0u).z) : (uvec4(8u).z));
  uvec4 v_16 = uvec4(v_13, v_14, v_15, ((v_12.w) ? (uvec4(0u).w) : (uvec4(8u).w)));
  bvec4 v_17 = equal((((v_6 >> v_11) >> v_16) & uvec4(240u)), uvec4(0u));
  uint v_18 = ((v_17.x) ? (uvec4(0u).x) : (uvec4(4u).x));
  uint v_19 = ((v_17.y) ? (uvec4(0u).y) : (uvec4(4u).y));
  uint v_20 = ((v_17.z) ? (uvec4(0u).z) : (uvec4(4u).z));
  uvec4 v_21 = uvec4(v_18, v_19, v_20, ((v_17.w) ? (uvec4(0u).w) : (uvec4(4u).w)));
  bvec4 v_22 = equal(((((v_6 >> v_11) >> v_16) >> v_21) & uvec4(12u)), uvec4(0u));
  uint v_23 = ((v_22.x) ? (uvec4(0u).x) : (uvec4(2u).x));
  uint v_24 = ((v_22.y) ? (uvec4(0u).y) : (uvec4(2u).y));
  uint v_25 = ((v_22.z) ? (uvec4(0u).z) : (uvec4(2u).z));
  uvec4 v_26 = uvec4(v_23, v_24, v_25, ((v_22.w) ? (uvec4(0u).w) : (uvec4(2u).w)));
  bvec4 v_27 = equal((((((v_6 >> v_11) >> v_16) >> v_21) >> v_26) & uvec4(2u)), uvec4(0u));
  uint v_28 = ((v_27.x) ? (uvec4(0u).x) : (uvec4(1u).x));
  uint v_29 = ((v_27.y) ? (uvec4(0u).y) : (uvec4(1u).y));
  uint v_30 = ((v_27.z) ? (uvec4(0u).z) : (uvec4(1u).z));
  uvec4 v_31 = (v_11 | (v_16 | (v_21 | (v_26 | uvec4(v_28, v_29, v_30, ((v_27.w) ? (uvec4(0u).w) : (uvec4(1u).w)))))));
  bvec4 v_32 = equal(((((v_6 >> v_11) >> v_16) >> v_21) >> v_26), uvec4(0u));
  uint v_33 = ((v_32.x) ? (uvec4(4294967295u).x) : (v_31.x));
  uint v_34 = ((v_32.y) ? (uvec4(4294967295u).y) : (v_31.y));
  uint v_35 = ((v_32.z) ? (uvec4(4294967295u).z) : (v_31.z));
  ivec4 res = ivec4(uvec4(v_33, v_34, v_35, ((v_32.w) ? (uvec4(4294967295u).w) : (v_31.w))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_c1f940();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 vertex_main_loc0_Output;
ivec4 firstLeadingBit_c1f940() {
  ivec4 arg_0 = ivec4(1);
  uvec4 v = uvec4(arg_0);
  bvec4 v_1 = lessThan(v, uvec4(2147483648u));
  uint v_2 = ((v_1.x) ? (v.x) : (~(v).x));
  uint v_3 = ((v_1.y) ? (v.y) : (~(v).y));
  uint v_4 = ((v_1.z) ? (v.z) : (~(v).z));
  uvec4 v_5 = uvec4(v_2, v_3, v_4, ((v_1.w) ? (v.w) : (~(v).w)));
  bvec4 v_6 = equal((v_5 & uvec4(4294901760u)), uvec4(0u));
  uint v_7 = ((v_6.x) ? (uvec4(0u).x) : (uvec4(16u).x));
  uint v_8 = ((v_6.y) ? (uvec4(0u).y) : (uvec4(16u).y));
  uint v_9 = ((v_6.z) ? (uvec4(0u).z) : (uvec4(16u).z));
  uvec4 v_10 = uvec4(v_7, v_8, v_9, ((v_6.w) ? (uvec4(0u).w) : (uvec4(16u).w)));
  bvec4 v_11 = equal(((v_5 >> v_10) & uvec4(65280u)), uvec4(0u));
  uint v_12 = ((v_11.x) ? (uvec4(0u).x) : (uvec4(8u).x));
  uint v_13 = ((v_11.y) ? (uvec4(0u).y) : (uvec4(8u).y));
  uint v_14 = ((v_11.z) ? (uvec4(0u).z) : (uvec4(8u).z));
  uvec4 v_15 = uvec4(v_12, v_13, v_14, ((v_11.w) ? (uvec4(0u).w) : (uvec4(8u).w)));
  bvec4 v_16 = equal((((v_5 >> v_10) >> v_15) & uvec4(240u)), uvec4(0u));
  uint v_17 = ((v_16.x) ? (uvec4(0u).x) : (uvec4(4u).x));
  uint v_18 = ((v_16.y) ? (uvec4(0u).y) : (uvec4(4u).y));
  uint v_19 = ((v_16.z) ? (uvec4(0u).z) : (uvec4(4u).z));
  uvec4 v_20 = uvec4(v_17, v_18, v_19, ((v_16.w) ? (uvec4(0u).w) : (uvec4(4u).w)));
  bvec4 v_21 = equal(((((v_5 >> v_10) >> v_15) >> v_20) & uvec4(12u)), uvec4(0u));
  uint v_22 = ((v_21.x) ? (uvec4(0u).x) : (uvec4(2u).x));
  uint v_23 = ((v_21.y) ? (uvec4(0u).y) : (uvec4(2u).y));
  uint v_24 = ((v_21.z) ? (uvec4(0u).z) : (uvec4(2u).z));
  uvec4 v_25 = uvec4(v_22, v_23, v_24, ((v_21.w) ? (uvec4(0u).w) : (uvec4(2u).w)));
  bvec4 v_26 = equal((((((v_5 >> v_10) >> v_15) >> v_20) >> v_25) & uvec4(2u)), uvec4(0u));
  uint v_27 = ((v_26.x) ? (uvec4(0u).x) : (uvec4(1u).x));
  uint v_28 = ((v_26.y) ? (uvec4(0u).y) : (uvec4(1u).y));
  uint v_29 = ((v_26.z) ? (uvec4(0u).z) : (uvec4(1u).z));
  uvec4 v_30 = (v_10 | (v_15 | (v_20 | (v_25 | uvec4(v_27, v_28, v_29, ((v_26.w) ? (uvec4(0u).w) : (uvec4(1u).w)))))));
  bvec4 v_31 = equal(((((v_5 >> v_10) >> v_15) >> v_20) >> v_25), uvec4(0u));
  uint v_32 = ((v_31.x) ? (uvec4(4294967295u).x) : (v_30.x));
  uint v_33 = ((v_31.y) ? (uvec4(4294967295u).y) : (v_30.y));
  uint v_34 = ((v_31.z) ? (uvec4(4294967295u).z) : (v_30.z));
  ivec4 res = ivec4(uvec4(v_32, v_33, v_34, ((v_31.w) ? (uvec4(4294967295u).w) : (v_30.w))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec4(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_c1f940();
  return tint_symbol;
}
void main() {
  VertexOutput v_35 = vertex_main_inner();
  gl_Position = v_35.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_35.prevent_dce;
  gl_PointSize = 1.0f;
}
