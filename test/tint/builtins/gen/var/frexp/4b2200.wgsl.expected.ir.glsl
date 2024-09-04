SKIP: FAILED

#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 res = frexp(arg_0);
}
void main() {
  frexp_4b2200();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_4b2200();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_4b2200();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 res = frexp(arg_0);
}
void main() {
  frexp_4b2200();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_4b2200();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_4b2200();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'frexp' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp float fract,  global highp int exp}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct frexp_result_f32 {
  float fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 res = frexp(arg_0);
}
void main() {
  frexp_4b2200();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_4b2200();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_4b2200();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'frexp' : no matching overloaded function found 
ERROR: 0:17: '=' :  cannot convert from ' const float' to ' temp structure{ global highp float fract,  global highp int exp}'
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
