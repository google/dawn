SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5257dd() {
  float16_t arg_0 = 1.0hf;
  frexp_result_f16 res = frexp(arg_0);
}
void main() {
  frexp_5257dd();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5257dd();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5257dd();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global mediump int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5257dd() {
  float16_t arg_0 = 1.0hf;
  frexp_result_f16 res = frexp(arg_0);
}
void main() {
  frexp_5257dd();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5257dd();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5257dd();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global highp int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_f16 {
  float16_t fract;
  int exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5257dd() {
  float16_t arg_0 = 1.0hf;
  frexp_result_f16 res = frexp(arg_0);
}
void main() {
  frexp_5257dd();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5257dd();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5257dd();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global highp int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
