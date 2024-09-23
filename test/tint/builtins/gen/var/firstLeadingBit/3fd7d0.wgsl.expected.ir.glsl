#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  bvec3 v_2 = equal((v_1 & uvec3(4294901760u)), uvec3(0u));
  uint v_3 = ((v_2.x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_4 = ((v_2.y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  bvec3 v_6 = equal(((v_1 >> v_5) & uvec3(65280u)), uvec3(0u));
  uint v_7 = ((v_6.x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_8 = ((v_6.y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_9 = uvec3(v_7, v_8, ((v_6.z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  bvec3 v_10 = equal((((v_1 >> v_5) >> v_9) & uvec3(240u)), uvec3(0u));
  uint v_11 = ((v_10.x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_12 = ((v_10.y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((v_10.z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  bvec3 v_14 = equal(((((v_1 >> v_5) >> v_9) >> v_13) & uvec3(12u)), uvec3(0u));
  uint v_15 = ((v_14.x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_16 = ((v_14.y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_17 = uvec3(v_15, v_16, ((v_14.z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  bvec3 v_18 = equal((((((v_1 >> v_5) >> v_9) >> v_13) >> v_17) & uvec3(2u)), uvec3(0u));
  uint v_19 = ((v_18.x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_20 = ((v_18.y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_21 = (v_5 | (v_9 | (v_13 | (v_17 | uvec3(v_19, v_20, ((v_18.z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  bvec3 v_22 = equal(((((v_1 >> v_5) >> v_9) >> v_13) >> v_17), uvec3(0u));
  uint v_23 = ((v_22.x) ? (uvec3(4294967295u).x) : (v_21.x));
  uint v_24 = ((v_22.y) ? (uvec3(4294967295u).y) : (v_21.y));
  uvec3 res = uvec3(v_23, v_24, ((v_22.z) ? (uvec3(4294967295u).z) : (v_21.z)));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_3fd7d0();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  bvec3 v_2 = equal((v_1 & uvec3(4294901760u)), uvec3(0u));
  uint v_3 = ((v_2.x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_4 = ((v_2.y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_5 = uvec3(v_3, v_4, ((v_2.z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  bvec3 v_6 = equal(((v_1 >> v_5) & uvec3(65280u)), uvec3(0u));
  uint v_7 = ((v_6.x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_8 = ((v_6.y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_9 = uvec3(v_7, v_8, ((v_6.z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  bvec3 v_10 = equal((((v_1 >> v_5) >> v_9) & uvec3(240u)), uvec3(0u));
  uint v_11 = ((v_10.x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_12 = ((v_10.y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((v_10.z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  bvec3 v_14 = equal(((((v_1 >> v_5) >> v_9) >> v_13) & uvec3(12u)), uvec3(0u));
  uint v_15 = ((v_14.x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_16 = ((v_14.y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_17 = uvec3(v_15, v_16, ((v_14.z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  bvec3 v_18 = equal((((((v_1 >> v_5) >> v_9) >> v_13) >> v_17) & uvec3(2u)), uvec3(0u));
  uint v_19 = ((v_18.x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_20 = ((v_18.y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_21 = (v_5 | (v_9 | (v_13 | (v_17 | uvec3(v_19, v_20, ((v_18.z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  bvec3 v_22 = equal(((((v_1 >> v_5) >> v_9) >> v_13) >> v_17), uvec3(0u));
  uint v_23 = ((v_22.x) ? (uvec3(4294967295u).x) : (v_21.x));
  uint v_24 = ((v_22.y) ? (uvec3(4294967295u).y) : (v_21.y));
  uvec3 res = uvec3(v_23, v_24, ((v_22.z) ? (uvec3(4294967295u).z) : (v_21.z)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_3fd7d0();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  bvec3 v_1 = equal((v & uvec3(4294901760u)), uvec3(0u));
  uint v_2 = ((v_1.x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_3 = ((v_1.y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_4 = uvec3(v_2, v_3, ((v_1.z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  bvec3 v_5 = equal(((v >> v_4) & uvec3(65280u)), uvec3(0u));
  uint v_6 = ((v_5.x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_7 = ((v_5.y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_8 = uvec3(v_6, v_7, ((v_5.z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  bvec3 v_9 = equal((((v >> v_4) >> v_8) & uvec3(240u)), uvec3(0u));
  uint v_10 = ((v_9.x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_11 = ((v_9.y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_12 = uvec3(v_10, v_11, ((v_9.z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  bvec3 v_13 = equal(((((v >> v_4) >> v_8) >> v_12) & uvec3(12u)), uvec3(0u));
  uint v_14 = ((v_13.x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_15 = ((v_13.y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_16 = uvec3(v_14, v_15, ((v_13.z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  bvec3 v_17 = equal((((((v >> v_4) >> v_8) >> v_12) >> v_16) & uvec3(2u)), uvec3(0u));
  uint v_18 = ((v_17.x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_19 = ((v_17.y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_20 = (v_4 | (v_8 | (v_12 | (v_16 | uvec3(v_18, v_19, ((v_17.z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  bvec3 v_21 = equal(((((v >> v_4) >> v_8) >> v_12) >> v_16), uvec3(0u));
  uint v_22 = ((v_21.x) ? (uvec3(4294967295u).x) : (v_20.x));
  uint v_23 = ((v_21.y) ? (uvec3(4294967295u).y) : (v_20.y));
  uvec3 res = uvec3(v_22, v_23, ((v_21.z) ? (uvec3(4294967295u).z) : (v_20.z)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_3fd7d0();
  return tint_symbol;
}
void main() {
  VertexOutput v_24 = vertex_main_inner();
  gl_Position = v_24.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_24.prevent_dce;
  gl_PointSize = 1.0f;
}
