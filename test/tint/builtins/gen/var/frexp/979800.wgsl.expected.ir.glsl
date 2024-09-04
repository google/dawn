SKIP: FAILED

#version 310 es

struct frexp_result_vec3_f32 {
  vec3 fract;
  ivec3 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_979800() {
  vec3 arg_0 = vec3(1.0f);
  frexp_result_vec3_f32 res = frexp(arg_0);
}
void main() {
  frexp_979800();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_979800();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_979800();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct frexp_result_vec3_f32 {
  vec3 fract;
  ivec3 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_979800() {
  vec3 arg_0 = vec3(1.0f);
  frexp_result_vec3_f32 res = frexp(arg_0);
}
void main() {
  frexp_979800();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_979800();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_979800();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'frexp' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 3-component vector of float fract,  global highp 3-component vector of int exp}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct frexp_result_vec3_f32 {
  vec3 fract;
  ivec3 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_979800() {
  vec3 arg_0 = vec3(1.0f);
  frexp_result_vec3_f32 res = frexp(arg_0);
}
void main() {
  frexp_979800();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_979800();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_979800();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'frexp' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp 3-component vector of float fract,  global highp 3-component vector of int exp}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
