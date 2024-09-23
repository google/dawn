SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uint v_2 = ((((v_1 & uvec3(65535u)) == uvec3(0u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_3 = ((((v_1 & uvec3(65535u)) == uvec3(0u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, ((((v_1 & uvec3(65535u)) == uvec3(0u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_5 = (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_6 = (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_7 = uvec3(v_5, v_6, (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_8 = ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_9 = ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_10 = uvec3(v_8, v_9, ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_11 = (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_12 = (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_14 = ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_15 = ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_16 = (v_4 | (v_7 | (v_10 | (v_13 | uvec3(v_14, v_15, ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)))))));
  uint v_17 = (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_16.x));
  uint v_18 = (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_16.y));
  uvec3 res = uvec3(v_17, v_18, (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_16.z)));
  return res;
}
void main() {
  v.tint_symbol = firstTrailingBit_cb51ce();
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uint v_2 = ((((v_1 & uvec3(65535u)) == uvec3(0u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_3 = ((((v_1 & uvec3(65535u)) == uvec3(0u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_4 = uvec3(v_2, v_3, ((((v_1 & uvec3(65535u)) == uvec3(0u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_5 = (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_6 = (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_7 = uvec3(v_5, v_6, (((((v_1 >> v_4) & uvec3(255u)) == uvec3(0u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_8 = ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_9 = ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_10 = uvec3(v_8, v_9, ((((((v_1 >> v_4) >> v_7) & uvec3(15u)) == uvec3(0u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_11 = (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_12 = (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_13 = uvec3(v_11, v_12, (((((((v_1 >> v_4) >> v_7) >> v_10) & uvec3(3u)) == uvec3(0u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_14 = ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_15 = ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_16 = (v_4 | (v_7 | (v_10 | (v_13 | uvec3(v_14, v_15, ((((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) & uvec3(1u)) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)))))));
  uint v_17 = (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_16.x));
  uint v_18 = (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_16.y));
  uvec3 res = uvec3(v_17, v_18, (((((((v_1 >> v_4) >> v_7) >> v_10) >> v_13) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_16.z)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstTrailingBit_cb51ce();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uint v_1 = ((((v & uvec3(65535u)) == uvec3(0u)).x) ? (uvec3(16u).x) : (uvec3(0u).x));
  uint v_2 = ((((v & uvec3(65535u)) == uvec3(0u)).y) ? (uvec3(16u).y) : (uvec3(0u).y));
  uvec3 v_3 = uvec3(v_1, v_2, ((((v & uvec3(65535u)) == uvec3(0u)).z) ? (uvec3(16u).z) : (uvec3(0u).z)));
  uint v_4 = (((((v >> v_3) & uvec3(255u)) == uvec3(0u)).x) ? (uvec3(8u).x) : (uvec3(0u).x));
  uint v_5 = (((((v >> v_3) & uvec3(255u)) == uvec3(0u)).y) ? (uvec3(8u).y) : (uvec3(0u).y));
  uvec3 v_6 = uvec3(v_4, v_5, (((((v >> v_3) & uvec3(255u)) == uvec3(0u)).z) ? (uvec3(8u).z) : (uvec3(0u).z)));
  uint v_7 = ((((((v >> v_3) >> v_6) & uvec3(15u)) == uvec3(0u)).x) ? (uvec3(4u).x) : (uvec3(0u).x));
  uint v_8 = ((((((v >> v_3) >> v_6) & uvec3(15u)) == uvec3(0u)).y) ? (uvec3(4u).y) : (uvec3(0u).y));
  uvec3 v_9 = uvec3(v_7, v_8, ((((((v >> v_3) >> v_6) & uvec3(15u)) == uvec3(0u)).z) ? (uvec3(4u).z) : (uvec3(0u).z)));
  uint v_10 = (((((((v >> v_3) >> v_6) >> v_9) & uvec3(3u)) == uvec3(0u)).x) ? (uvec3(2u).x) : (uvec3(0u).x));
  uint v_11 = (((((((v >> v_3) >> v_6) >> v_9) & uvec3(3u)) == uvec3(0u)).y) ? (uvec3(2u).y) : (uvec3(0u).y));
  uvec3 v_12 = uvec3(v_10, v_11, (((((((v >> v_3) >> v_6) >> v_9) & uvec3(3u)) == uvec3(0u)).z) ? (uvec3(2u).z) : (uvec3(0u).z)));
  uint v_13 = ((((((((v >> v_3) >> v_6) >> v_9) >> v_12) & uvec3(1u)) == uvec3(0u)).x) ? (uvec3(1u).x) : (uvec3(0u).x));
  uint v_14 = ((((((((v >> v_3) >> v_6) >> v_9) >> v_12) & uvec3(1u)) == uvec3(0u)).y) ? (uvec3(1u).y) : (uvec3(0u).y));
  uvec3 v_15 = (v_3 | (v_6 | (v_9 | (v_12 | uvec3(v_13, v_14, ((((((((v >> v_3) >> v_6) >> v_9) >> v_12) & uvec3(1u)) == uvec3(0u)).z) ? (uvec3(1u).z) : (uvec3(0u).z)))))));
  uint v_16 = (((((((v >> v_3) >> v_6) >> v_9) >> v_12) == uvec3(0u)).x) ? (uvec3(4294967295u).x) : (v_15.x));
  uint v_17 = (((((((v >> v_3) >> v_6) >> v_9) >> v_12) == uvec3(0u)).y) ? (uvec3(4294967295u).y) : (v_15.y));
  uvec3 res = uvec3(v_16, v_17, (((((((v >> v_3) >> v_6) >> v_9) >> v_12) == uvec3(0u)).z) ? (uvec3(4294967295u).z) : (v_15.z)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_cb51ce();
  return tint_symbol;
}
void main() {
  VertexOutput v_18 = vertex_main_inner();
  gl_Position = v_18.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_18.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
