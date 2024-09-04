SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

int prevent_dce;
int dot_ef6b1d() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  int res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_ef6b1d();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_ef6b1d();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_ef6b1d();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

int prevent_dce;
int dot_ef6b1d() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  int res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_ef6b1d();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_ef6b1d();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_ef6b1d();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

int prevent_dce;
int dot_ef6b1d() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  int res = dot(arg_0, arg_1);
  return res;
}
void main() {
  prevent_dce = dot_ef6b1d();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  prevent_dce = dot_ef6b1d();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = dot_ef6b1d();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'dot' : no matching overloaded function found 
ERROR: 0:15: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
