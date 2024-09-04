SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 res = frexp(arg_0);
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global mediump 2-component vector of int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 res = frexp(arg_0);
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global highp 2-component vector of int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct frexp_result_vec2_f16 {
  f16vec2 fract;
  ivec2 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_5f47bf() {
  f16vec2 arg_0 = f16vec2(1.0hf);
  frexp_result_vec2_f16 res = frexp(arg_0);
}
void main() {
  frexp_5f47bf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_5f47bf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_5f47bf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'frexp' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global highp 2-component vector of int exp}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
