SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_4 = (uvec4(v_1) >> v_3);
  uvec4 v_5 = (v_4 & uvec4(255u));
  uvec4 v_6 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_7 = (uvec4(v_2) >> v_6);
  uint res = dot(v_5, (v_7 & uvec4(255u)));
  return res;
}
void main() {
  v.tint_symbol = dot4U8Packed_fbed7b();
}
error: Error parsing GLSL shader:
ERROR: 0:19: 'dot' : no matching overloaded function found 
ERROR: 0:19: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:19: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_4 = (uvec4(v_1) >> v_3);
  uvec4 v_5 = (v_4 & uvec4(255u));
  uvec4 v_6 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_7 = (uvec4(v_2) >> v_6);
  uint res = dot(v_5, (v_7 & uvec4(255u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = dot4U8Packed_fbed7b();
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'dot' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v) >> v_2);
  uvec4 v_4 = (v_3 & uvec4(255u));
  uvec4 v_5 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_6 = (uvec4(v_1) >> v_5);
  uint res = dot(v_4, (v_6 & uvec4(255u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
  return tint_symbol;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = v_7.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:20: 'dot' : no matching overloaded function found 
ERROR: 0:20: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:20: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
