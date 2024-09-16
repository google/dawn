SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
vec4 select_713567() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  bool arg_2 = true;
  vec4 v_1 = arg_0;
  vec4 v_2 = arg_1;
  bool v_3 = arg_2;
  float v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  float v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  float v_6 = ((v_3.z) ? (v_2.z) : (v_1.z));
  vec4 res = vec4(v_4, v_5, v_6, ((v_3.w) ? (v_2.w) : (v_1.w)));
  return res;
}
void main() {
  v.tint_symbol = select_713567();
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
vec4 select_713567() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  bool arg_2 = true;
  vec4 v_1 = arg_0;
  vec4 v_2 = arg_1;
  bool v_3 = arg_2;
  float v_4 = ((v_3.x) ? (v_2.x) : (v_1.x));
  float v_5 = ((v_3.y) ? (v_2.y) : (v_1.y));
  float v_6 = ((v_3.z) ? (v_2.z) : (v_1.z));
  vec4 res = vec4(v_4, v_5, v_6, ((v_3.w) ? (v_2.w) : (v_1.w)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_713567();
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 select_713567() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  bool arg_2 = true;
  vec4 v = arg_0;
  vec4 v_1 = arg_1;
  bool v_2 = arg_2;
  float v_3 = ((v_2.x) ? (v_1.x) : (v.x));
  float v_4 = ((v_2.y) ? (v_1.y) : (v.y));
  float v_5 = ((v_2.z) ? (v_1.z) : (v.z));
  vec4 res = vec4(v_3, v_4, v_5, ((v_2.w) ? (v_1.w) : (v.w)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_713567();
  return tint_symbol;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = v_6.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'scalar swizzle' : not supported with this profile: es
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
