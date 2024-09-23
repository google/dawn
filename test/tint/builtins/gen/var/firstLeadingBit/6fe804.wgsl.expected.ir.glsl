SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uvec2 firstLeadingBit_6fe804() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uint v_2 = ((((v_1 & uvec2(4294901760u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(16u).x));
  uvec2 v_3 = uvec2(v_2, ((((v_1 & uvec2(4294901760u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(16u).y)));
  uint v_4 = (((((v_1 >> v_3) & uvec2(65280u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(8u).x));
  uvec2 v_5 = uvec2(v_4, (((((v_1 >> v_3) & uvec2(65280u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(8u).y)));
  uint v_6 = ((((((v_1 >> v_3) >> v_5) & uvec2(240u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(4u).x));
  uvec2 v_7 = uvec2(v_6, ((((((v_1 >> v_3) >> v_5) & uvec2(240u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(4u).y)));
  uint v_8 = (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(12u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(2u).x));
  uvec2 v_9 = uvec2(v_8, (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(12u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(2u).y)));
  uint v_10 = ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(2u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(1u).x));
  uvec2 v_11 = (v_3 | (v_5 | (v_7 | (v_9 | uvec2(v_10, ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(2u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(1u).y)))))));
  uint v_12 = (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_11.x));
  uvec2 res = uvec2(v_12, (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_11.y)));
  return res;
}
void main() {
  v.tint_symbol = firstLeadingBit_6fe804();
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec2 tint_symbol;
} v;
uvec2 firstLeadingBit_6fe804() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uint v_2 = ((((v_1 & uvec2(4294901760u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(16u).x));
  uvec2 v_3 = uvec2(v_2, ((((v_1 & uvec2(4294901760u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(16u).y)));
  uint v_4 = (((((v_1 >> v_3) & uvec2(65280u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(8u).x));
  uvec2 v_5 = uvec2(v_4, (((((v_1 >> v_3) & uvec2(65280u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(8u).y)));
  uint v_6 = ((((((v_1 >> v_3) >> v_5) & uvec2(240u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(4u).x));
  uvec2 v_7 = uvec2(v_6, ((((((v_1 >> v_3) >> v_5) & uvec2(240u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(4u).y)));
  uint v_8 = (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(12u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(2u).x));
  uvec2 v_9 = uvec2(v_8, (((((((v_1 >> v_3) >> v_5) >> v_7) & uvec2(12u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(2u).y)));
  uint v_10 = ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(2u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(1u).x));
  uvec2 v_11 = (v_3 | (v_5 | (v_7 | (v_9 | uvec2(v_10, ((((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) & uvec2(2u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(1u).y)))))));
  uint v_12 = (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_11.x));
  uvec2 res = uvec2(v_12, (((((((v_1 >> v_3) >> v_5) >> v_7) >> v_9) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_11.y)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = firstLeadingBit_6fe804();
}
error: Error parsing GLSL shader:
ERROR: 0:10: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(location = 0) flat out uvec2 vertex_main_loc0_Output;
uvec2 firstLeadingBit_6fe804() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v = arg_0;
  uint v_1 = ((((v & uvec2(4294901760u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(16u).x));
  uvec2 v_2 = uvec2(v_1, ((((v & uvec2(4294901760u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(16u).y)));
  uint v_3 = (((((v >> v_2) & uvec2(65280u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(8u).x));
  uvec2 v_4 = uvec2(v_3, (((((v >> v_2) & uvec2(65280u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(8u).y)));
  uint v_5 = ((((((v >> v_2) >> v_4) & uvec2(240u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(4u).x));
  uvec2 v_6 = uvec2(v_5, ((((((v >> v_2) >> v_4) & uvec2(240u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(4u).y)));
  uint v_7 = (((((((v >> v_2) >> v_4) >> v_6) & uvec2(12u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(2u).x));
  uvec2 v_8 = uvec2(v_7, (((((((v >> v_2) >> v_4) >> v_6) & uvec2(12u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(2u).y)));
  uint v_9 = ((((((((v >> v_2) >> v_4) >> v_6) >> v_8) & uvec2(2u)) == uvec2(0u)).x) ? (uvec2(0u).x) : (uvec2(1u).x));
  uvec2 v_10 = (v_2 | (v_4 | (v_6 | (v_8 | uvec2(v_9, ((((((((v >> v_2) >> v_4) >> v_6) >> v_8) & uvec2(2u)) == uvec2(0u)).y) ? (uvec2(0u).y) : (uvec2(1u).y)))))));
  uint v_11 = (((((((v >> v_2) >> v_4) >> v_6) >> v_8) == uvec2(0u)).x) ? (uvec2(4294967295u).x) : (v_10.x));
  uvec2 res = uvec2(v_11, (((((((v >> v_2) >> v_4) >> v_6) >> v_8) == uvec2(0u)).y) ? (uvec2(4294967295u).y) : (v_10.y)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec2(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstLeadingBit_6fe804();
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
