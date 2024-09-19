SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = uvec4(v_1);
  uvec4 v_4 = ((v_3 & uvec4(255u)) << v_2);
  uint res = dot(v_4, uvec4(1u));
  return res;
}
void main() {
  v.tint_symbol = pack4xI8_bfce01();
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = uvec4(v_1);
  uvec4 v_4 = ((v_3 & uvec4(255u)) << v_2);
  uint res = dot(v_4, uvec4(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = pack4xI8_bfce01();
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'dot' : no matching overloaded function found 
ERROR: 0:13: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint vertex_main_loc0_Output;
uint pack4xI8_bfce01() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_2 = uvec4(v);
  uvec4 v_3 = ((v_2 & uvec4(255u)) << v_1);
  uint res = dot(v_3, uvec4(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = pack4xI8_bfce01();
  return tint_symbol;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = v_4.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'dot' : no matching overloaded function found 
ERROR: 0:16: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
