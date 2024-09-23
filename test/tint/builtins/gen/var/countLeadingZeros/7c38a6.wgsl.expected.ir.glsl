#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  bvec3 v_2 = lessThanEqual(v_1, uvec3(65535u));
  uint v_3 = ((v_2.x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_4 = ((v_2.y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  bvec3 v_6 = lessThanEqual((v_1 << v_5), uvec3(16777215u));
  uint v_7 = ((v_6.x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_8 = ((v_6.y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_9 = uvec3(v_7, v_8, ((v_6.z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  bvec3 v_10 = lessThanEqual(((v_1 << v_5) << v_9), uvec3(268435455u));
  uint v_11 = ((v_10.x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_12 = ((v_10.y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((v_10.z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  bvec3 v_14 = lessThanEqual((((v_1 << v_5) << v_9) << v_13), uvec3(1073741823u));
  uint v_15 = ((v_14.x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_16 = ((v_14.y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_17 = uvec3(v_15, v_16, ((v_14.z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  bvec3 v_18 = lessThanEqual(((((v_1 << v_5) << v_9) << v_13) << v_17), uvec3(2147483647u));
  uint v_19 = ((v_18.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_20 = ((v_18.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_21 = uvec3(v_19, v_20, ((v_18.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  bvec3 v_22 = equal(((((v_1 << v_5) << v_9) << v_13) << v_17), uvec3(0u));
  uint v_23 = ((v_22.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_24 = ((v_22.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_25 = uvec3(v_23, v_24, ((v_22.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_5 | (v_9 | (v_13 | (v_17 | (v_21 | v_25))))) + v_25));
  return res;
}
void main() {
  v.tint_symbol = countLeadingZeros_7c38a6();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  bvec3 v_2 = lessThanEqual(v_1, uvec3(65535u));
  uint v_3 = ((v_2.x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_4 = ((v_2.y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  bvec3 v_6 = lessThanEqual((v_1 << v_5), uvec3(16777215u));
  uint v_7 = ((v_6.x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_8 = ((v_6.y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_9 = uvec3(v_7, v_8, ((v_6.z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  bvec3 v_10 = lessThanEqual(((v_1 << v_5) << v_9), uvec3(268435455u));
  uint v_11 = ((v_10.x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_12 = ((v_10.y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((v_10.z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  bvec3 v_14 = lessThanEqual((((v_1 << v_5) << v_9) << v_13), uvec3(1073741823u));
  uint v_15 = ((v_14.x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_16 = ((v_14.y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_17 = uvec3(v_15, v_16, ((v_14.z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  bvec3 v_18 = lessThanEqual(((((v_1 << v_5) << v_9) << v_13) << v_17), uvec3(2147483647u));
  uint v_19 = ((v_18.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_20 = ((v_18.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_21 = uvec3(v_19, v_20, ((v_18.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  bvec3 v_22 = equal(((((v_1 << v_5) << v_9) << v_13) << v_17), uvec3(0u));
  uint v_23 = ((v_22.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_24 = ((v_22.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_25 = uvec3(v_23, v_24, ((v_22.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_5 | (v_9 | (v_13 | (v_17 | (v_21 | v_25))))) + v_25));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countLeadingZeros_7c38a6();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v = uvec3(arg_0);
  bvec3 v_1 = lessThanEqual(v, uvec3(65535u));
  uint v_2 = ((v_1.x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_3 = ((v_1.y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, ((v_1.z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  bvec3 v_5 = lessThanEqual((v << v_4), uvec3(16777215u));
  uint v_6 = ((v_5.x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_7 = ((v_5.y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_8 = uvec3(v_6, v_7, ((v_5.z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  bvec3 v_9 = lessThanEqual(((v << v_4) << v_8), uvec3(268435455u));
  uint v_10 = ((v_9.x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_11 = ((v_9.y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_12 = uvec3(v_10, v_11, ((v_9.z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  bvec3 v_13 = lessThanEqual((((v << v_4) << v_8) << v_12), uvec3(1073741823u));
  uint v_14 = ((v_13.x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_15 = ((v_13.y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_16 = uvec3(v_14, v_15, ((v_13.z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  bvec3 v_17 = lessThanEqual(((((v << v_4) << v_8) << v_12) << v_16), uvec3(2147483647u));
  uint v_18 = ((v_17.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_19 = ((v_17.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_20 = uvec3(v_18, v_19, ((v_17.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  bvec3 v_21 = equal(((((v << v_4) << v_8) << v_12) << v_16), uvec3(0u));
  uint v_22 = ((v_21.x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_23 = ((v_21.y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_24 = uvec3(v_22, v_23, ((v_21.z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_4 | (v_8 | (v_12 | (v_16 | (v_20 | v_24))))) + v_24));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_7c38a6();
  return tint_symbol;
}
void main() {
  VertexOutput v_25 = vertex_main_inner();
  gl_Position = v_25.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_25.prevent_dce;
  gl_PointSize = 1.0f;
}
