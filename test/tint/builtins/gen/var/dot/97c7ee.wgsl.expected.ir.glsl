SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_97c7ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_97c7ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_97c7ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_97c7ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_97c7ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_97c7ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uint prevent_dce;
uint dot_97c7ee() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 arg_1 = uvec2(1u);
  uint res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_97c7ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_97c7ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_97c7ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp uint'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
