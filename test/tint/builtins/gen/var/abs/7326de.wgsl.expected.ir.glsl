SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 abs_7326de() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = abs(arg_0);
  return res;
}
void main() {
  v.tint_symbol = abs_7326de();
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'abs' : no matching overloaded function found 
ERROR: 0:11: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of uint'
ERROR: 0:11: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uvec3 abs_7326de() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = abs(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = abs_7326de();
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'abs' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of uint'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 abs_7326de() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 res = abs(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = abs_7326de();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'abs' : no matching overloaded function found 
ERROR: 0:12: '=' :  cannot convert from ' const float' to ' temp highp 3-component vector of uint'
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
