SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 firstTrailingBit_50c072() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v_1 = uvec2(arg_0);
  uint v_2 = ((((v_1 & uvec2(65535u)) == uvec2(0u)).x) ? (uvec2(16u).x) : (uvec2(0u).x));
  uvec2 v_3 = uvec2(v_2, ((((v_1 & uvec2(65535u)) == uvec2(0u)).y) ? (uvec2(16u).y) : (uvec2(0u).y)));
  uint v_4 = (((((v_1 >> v_3) & uvec2(255u)) == uvec2(0u)).x) ? (uvec2(8u).x) : (uvec2(0u).x));
  uvec2 v_5 = uvec2(v_4, (((((v_1 >> v_3) & uvec2(255u)) == uvec2(0u)).y) ? (uvec2(8u).y) : (uvec2(0u).y)));
  uint v_6 = ((((((v_1 >> v_3) >> v_5) & uvec2(15u)) == uvec2(0u)).x) ? (uvec2(4u).x) : (uvec2(0u).x));
  uvec2 v_7 = uvec2(v_6, ((((((v_1 >> v_3) >> v_5) & uvec2(15u)) == uvec2(0u)).y) ? (uvec2(4u).y) : (uvec2(0u).y)));
  uint v_8 = (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(3u)) == uvec2(0u)).x) ? (uvec2(2u).x) : (uvec2(0u).x));
  uvec2 v_9 = uvec2(v_8, (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(3u)) == uvec2(0u)).y) ? (uvec2(2u).y) : (uvec2(0u).y)));
  uint v_10 = ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(1u)) == uvec2(0u)).x) ? (uvec2(1u).x) : (uvec2(0u).x));
  uvec2 v_11 = (v_3 | (v_5 | (v_7 | (v_9 | uvec2(v_10, ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(1u)) == uvec2(0u)).y) ? (uvec2(1u).y) : (uvec2(0u).y)))))));
  uint v_12 = (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_11.x));
  ivec2 res = ivec2(uvec2(v_12, (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_11.y))));
  return res;
}
void main() {
  v.tint_symbol = firstTrailingBit_50c072();
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 firstTrailingBit_50c072() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v_1 = uvec2(arg_0);
  uint v_2 = ((((v_1 & uvec2(65535u)) == uvec2(0u)).x) ? (uvec2(16u).x) : (uvec2(0u).x));
  uvec2 v_3 = uvec2(v_2, ((((v_1 & uvec2(65535u)) == uvec2(0u)).y) ? (uvec2(16u).y) : (uvec2(0u).y)));
  uint v_4 = (((((v_1 >> v_3) & uvec2(255u)) == uvec2(0u)).x) ? (uvec2(8u).x) : (uvec2(0u).x));
  uvec2 v_5 = uvec2(v_4, (((((v_1 >> v_3) & uvec2(255u)) == uvec2(0u)).y) ? (uvec2(8u).y) : (uvec2(0u).y)));
  uint v_6 = ((((((v_1 >> v_3) >> v_5) & uvec2(15u)) == uvec2(0u)).x) ? (uvec2(4u).x) : (uvec2(0u).x));
  uvec2 v_7 = uvec2(v_6, ((((((v_1 >> v_3) >> v_5) & uvec2(15u)) == uvec2(0u)).y) ? (uvec2(4u).y) : (uvec2(0u).y)));
  uint v_8 = (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(3u)) == uvec2(0u)).x) ? (uvec2(2u).x) : (uvec2(0u).x));
  uvec2 v_9 = uvec2(v_8, (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(3u)) == uvec2(0u)).y) ? (uvec2(2u).y) : (uvec2(0u).y)));
  uint v_10 = ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(1u)) == uvec2(0u)).x) ? (uvec2(1u).x) : (uvec2(0u).x));
  uvec2 v_11 = (v_3 | (v_5 | (v_7 | (v_9 | uvec2(v_10, ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(1u)) == uvec2(0u)).y) ? (uvec2(1u).y) : (uvec2(0u).y)))))));
  uint v_12 = (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_11.x));
  ivec2 res = ivec2(uvec2(v_12, (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_11.y))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstTrailingBit_50c072();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec2 prevent_dce;
};

layout(location = 0) flat out ivec2 vertex_main_loc0_Output;
ivec2 firstTrailingBit_50c072() {
  ivec2 arg_0 = ivec2(1);
  uvec2 v = uvec2(arg_0);
  uint v_1 = ((((v & uvec2(65535u)) == uvec2(0u)).x) ? (uvec2(16u).x) : (uvec2(0u).x));
  uvec2 v_2 = uvec2(v_1, ((((v & uvec2(65535u)) == uvec2(0u)).y) ? (uvec2(16u).y) : (uvec2(0u).y)));
  uint v_3 = (((((v >> v_2) & uvec2(255u)) == uvec2(0u)).x) ? (uvec2(8u).x) : (uvec2(0u).x));
  uvec2 v_4 = uvec2(v_3, (((((v >> v_2) & uvec2(255u)) == uvec2(0u)).y) ? (uvec2(8u).y) : (uvec2(0u).y)));
  uint v_5 = ((((((v >> v_2) >> v_4) & uvec2(15u)) == uvec2(0u)).x) ? (uvec2(4u).x) : (uvec2(0u).x));
  uvec2 v_6 = uvec2(v_5, ((((((v >> v_2) >> v_4) & uvec2(15u)) == uvec2(0u)).y) ? (uvec2(4u).y) : (uvec2(0u).y)));
  uint v_7 = (((((((v >> v_2) >> v_4) >> v_6) & uvec2(3u)) == uvec2(0u)).x) ? (uvec2(2u).x) : (uvec2(0u).x));
  uvec2 v_8 = uvec2(v_7, (((((((v >> v_2) >> v_4) >> v_6) & uvec2(3u)) == uvec2(0u)).y) ? (uvec2(2u).y) : (uvec2(0u).y)));
  uint v_9 = ((((((((v >> v_2) >> v_4) >> v_6) >> v_8) & uvec2(1u)) == uvec2(0u)).x) ? (uvec2(1u).x) : (uvec2(0u).x));
  uvec2 v_10 = (v_2 | (v_4 | (v_6 | (v_8 | uvec2(v_9, ((((((((v >> v_2) >> v_4) >> v_6) >> v_8) & uvec2(1u)) == uvec2(0u)).y) ? (uvec2(1u).y) : (uvec2(0u).y)))))));
  uint v_11 = (((((((v >> v_2) >> v_4) >> v_6) >> v_8) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_10.x));
  ivec2 res = ivec2(uvec2(v_11, (((((((v >> v_2) >> v_4) >> v_6) >> v_8) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_10.y))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec2(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_50c072();
  return tint_symbol;
}
void main() {
  VertexOutput v_12 = vertex_main_inner();
  gl_Position = v_12.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_12.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
