SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 res = modf(arg_0);
}
void main() {
  modf_8dbbbf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_8dbbbf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_8dbbbf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'modf' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global float16_t whole}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 res = modf(arg_0);
}
void main() {
  modf_8dbbbf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_8dbbbf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_8dbbbf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'modf' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global float16_t whole}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 res = modf(arg_0);
}
void main() {
  modf_8dbbbf();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_8dbbbf();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_8dbbbf();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'modf' : no matching overloaded function found 
ERROR: 0:18: '=' :  cannot convert from ' const float' to ' temp structure{ global float16_t fract,  global float16_t whole}'
ERROR: 0:18: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
