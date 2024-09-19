SKIP: FAILED

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
  uint v_2 = (((v_1 <= uvec3(65535u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_3 = (((v_1 <= uvec3(65535u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((v_1 <= uvec3(65535u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_5 = ((((v_1 << v_4) <= uvec3(16777215u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_6 = ((((v_1 << v_4) <= uvec3(16777215u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_7 = uvec3(v_5, v_6, ((((v_1 << v_4) <= uvec3(16777215u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_8 = (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_9 = (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_10 = uvec3(v_8, v_9, (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_11 = ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_12 = ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_14 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_15 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_16 = uvec3(v_14, v_15, (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  uint v_17 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_18 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_19 = uvec3(v_17, v_18, (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_4 | (v_7 | (v_10 | (v_13 | (v_16 | v_19))))) + v_19));
  return res;
}
void main() {
  v.tint_symbol = countLeadingZeros_7c38a6();
}
error: Error parsing GLSL shader:
ERROR: 0:12: '<=' :  wrong operand types: no operation '<=' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec3 tint_symbol;
} v;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uint v_2 = (((v_1 <= uvec3(65535u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_3 = (((v_1 <= uvec3(65535u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, (((v_1 <= uvec3(65535u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_5 = ((((v_1 << v_4) <= uvec3(16777215u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_6 = ((((v_1 << v_4) <= uvec3(16777215u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_7 = uvec3(v_5, v_6, ((((v_1 << v_4) <= uvec3(16777215u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_8 = (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_9 = (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_10 = uvec3(v_8, v_9, (((((v_1 << v_4) << v_7) <= uvec3(268435455u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_11 = ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_12 = ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, ((((((v_1 << v_4) << v_7) << v_10) <= uvec3(1073741823u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_14 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_15 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_16 = uvec3(v_14, v_15, (((((((v_1 << v_4) << v_7) << v_10) << v_13) <= uvec3(2147483647u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  uint v_17 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_18 = (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_19 = uvec3(v_17, v_18, (((((((v_1 << v_4) << v_7) << v_10) << v_13) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_4 | (v_7 | (v_10 | (v_13 | (v_16 | v_19))))) + v_19));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = countLeadingZeros_7c38a6();
}
error: Error parsing GLSL shader:
ERROR: 0:10: '<=' :  wrong operand types: no operation '<=' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 countLeadingZeros_7c38a6() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v = uvec3(arg_0);
  uint v_1 = (((v <= uvec3(65535u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_2 = (((v <= uvec3(65535u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_3 = uvec3(v_1, v_2, (((v <= uvec3(65535u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_4 = ((((v << v_3) <= uvec3(16777215u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_5 = ((((v << v_3) <= uvec3(16777215u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_6 = uvec3(v_4, v_5, ((((v << v_3) <= uvec3(16777215u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_7 = (((((v << v_3) << v_6) <= uvec3(268435455u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_8 = (((((v << v_3) << v_6) <= uvec3(268435455u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_9 = uvec3(v_7, v_8, (((((v << v_3) << v_6) <= uvec3(268435455u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_10 = ((((((v << v_3) << v_6) << v_9) <= uvec3(1073741823u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_11 = ((((((v << v_3) << v_6) << v_9) <= uvec3(1073741823u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_12 = uvec3(v_10, v_11, ((((((v << v_3) << v_6) << v_9) <= uvec3(1073741823u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_13 = (((((((v << v_3) << v_6) << v_9) << v_12) <= uvec3(2147483647u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_14 = (((((((v << v_3) << v_6) << v_9) << v_12) <= uvec3(2147483647u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_15 = uvec3(v_13, v_14, (((((((v << v_3) << v_6) << v_9) << v_12) <= uvec3(2147483647u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  uint v_16 = (((((((v << v_3) << v_6) << v_9) << v_12) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_17 = (((((((v << v_3) << v_6) << v_9) << v_12) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_18 = uvec3(v_16, v_17, (((((((v << v_3) << v_6) << v_9) << v_12) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)));
  ivec3 res = ivec3(((v_3 | (v_6 | (v_9 | (v_12 | (v_15 | v_18))))) + v_18));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countLeadingZeros_7c38a6();
  return tint_symbol;
}
void main() {
  VertexOutput v_19 = vertex_main_inner();
  gl_Position = v_19.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_19.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:13: '<=' :  wrong operand types: no operation '<=' exists that takes a left-hand operand of type ' temp highp 3-component vector of uint' and a right operand of type ' const 3-component vector of uint' (or there is no acceptable conversion)
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
