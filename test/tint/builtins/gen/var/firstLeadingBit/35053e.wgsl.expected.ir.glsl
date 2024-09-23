SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uint v_2 = (((v_1 < uvec3(2147483648u)).x) ? (v_1.x) : (~(v_1).x));
  uint v_3 = (((v_1 < uvec3(2147483648u)).y) ? (v_1.y) : (~(v_1).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((v_1 < uvec3(2147483648u)).z) ? (v_1.z) : (~(v_1).z)));
  uint v_5 = ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_6 = ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_7 = uvec3(v_5, v_6, ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  uint v_8 = (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_9 = (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_10 = uvec3(v_8, v_9, (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  uint v_11 = ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_12 = ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  uint v_14 = (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_15 = (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_16 = uvec3(v_14, v_15, (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  uint v_17 = ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_18 = ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_19 = (v_7 | (v_10 | (v_13 | (v_16 | uvec3(v_17, v_18, ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  uint v_20 = (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_19.x));
  uint v_21 = (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_19.y));
  ivec3 res = ivec3(uvec3(v_20, v_21, (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_19.z))));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_35053e();
}
error: Error parsing GLSL shader:
ERROR: 0:12: '<' :  wrong operand types: no operation '<' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uint v_2 = (((v_1 < uvec3(2147483648u)).x) ? (v_1.x) : (~(v_1).x));
  uint v_3 = (((v_1 < uvec3(2147483648u)).y) ? (v_1.y) : (~(v_1).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((v_1 < uvec3(2147483648u)).z) ? (v_1.z) : (~(v_1).z)));
  uint v_5 = ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_6 = ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_7 = uvec3(v_5, v_6, ((((v_4 & uvec3(4294901760u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  uint v_8 = (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_9 = (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_10 = uvec3(v_8, v_9, (((((v_4 >> v_7) & uvec3(65280u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  uint v_11 = ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_12 = ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((((((v_4 >> v_7) >> v_10) & uvec3(240u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  uint v_14 = (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_15 = (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_16 = uvec3(v_14, v_15, (((((((v_4 >> v_7) >> v_10) >> v_13) & uvec3(12u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  uint v_17 = ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_18 = ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_19 = (v_7 | (v_10 | (v_13 | (v_16 | uvec3(v_17, v_18, ((((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) & uvec3(2u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  uint v_20 = (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_19.x));
  uint v_21 = (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_19.y));
  ivec3 res = ivec3(uvec3(v_20, v_21, (((((((v_4 >> v_7) >> v_10) >> v_13) >> v_16) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_19.z))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_35053e();
}
error: Error parsing GLSL shader:
ERROR: 0:10: '<' :  wrong operand types: no operation '<' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 firstLeadingBit_35053e() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v = uvec3(arg_0);
  uint v_1 = (((v < uvec3(2147483648u)).x) ? (v.x) : (~(v).x));
  uint v_2 = (((v < uvec3(2147483648u)).y) ? (v.y) : (~(v).y));
  uvec3 v_3 = uvec3(v_1, v_2, (((v < uvec3(2147483648u)).z) ? (v.z) : (~(v).z)));
  uint v_4 = ((((v_3 & uvec3(4294901760u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(16u).x));
  uint v_5 = ((((v_3 & uvec3(4294901760u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(16u).y));
  uvec3 v_6 = uvec3(v_4, v_5, ((((v_3 & uvec3(4294901760u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(16u).z)));
  uint v_7 = (((((v_3 >> v_6) & uvec3(65280u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(8u).x));
  uint v_8 = (((((v_3 >> v_6) & uvec3(65280u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(8u).y));
  uvec3 v_9 = uvec3(v_7, v_8, (((((v_3 >> v_6) & uvec3(65280u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(8u).z)));
  uint v_10 = ((((((v_3 >> v_6) >> v_9) & uvec3(240u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(4u).x));
  uint v_11 = ((((((v_3 >> v_6) >> v_9) & uvec3(240u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(4u).y));
  uvec3 v_12 = uvec3(v_10, v_11, ((((((v_3 >> v_6) >> v_9) & uvec3(240u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(4u).z)));
  uint v_13 = (((((((v_3 >> v_6) >> v_9) >> v_12) & uvec3(12u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(2u).x));
  uint v_14 = (((((((v_3 >> v_6) >> v_9) >> v_12) & uvec3(12u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(2u).y));
  uvec3 v_15 = uvec3(v_13, v_14, (((((((v_3 >> v_6) >> v_9) >> v_12) & uvec3(12u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(2u).z)));
  uint v_16 = ((((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) & uvec3(2u)) == uvec3(0u)).x) ? (uvec3(0u).x) : (uvec3(1u).x));
  uint v_17 = ((((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) & uvec3(2u)) == uvec3(0u)).y) ? (uvec3(0u).y) : (uvec3(1u).y));
  uvec3 v_18 = (v_6 | (v_9 | (v_12 | (v_15 | uvec3(v_16, v_17, ((((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) & uvec3(2u)) == uvec3(0u)).z) ? (uvec3(0u).z) : (uvec3(1u).z)))))));
  uint v_19 = (((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_18.x));
  uint v_20 = (((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_18.y));
  ivec3 res = ivec3(uvec3(v_19, v_20, (((((((v_3 >> v_6) >> v_9) >> v_12) >> v_15) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_18.z))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_35053e();
  return tint_symbol;
}
void main() {
  VertexOutput v_21 = vertex_main_inner();
  gl_Position = v_21.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_21.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:13: '<' :  wrong operand types: no operation '<' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
