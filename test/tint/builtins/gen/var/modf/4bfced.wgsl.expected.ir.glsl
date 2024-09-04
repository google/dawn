SKIP: FAILED

#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
void main() {
  modf_4bfced();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_4bfced();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_4bfced();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
void main() {
  modf_4bfced();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_4bfced();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_4bfced();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'modf' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 4-component vector of float fract,  global highp 4-component vector of float whole}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_4bfced() {
  vec4 arg_0 = vec4(-1.5f);
  modf_result_vec4_f32 res = modf(arg_0);
}
void main() {
  modf_4bfced();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_4bfced();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_4bfced();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'modf' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 4-component vector of float fract,  global highp 4-component vector of float whole}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
