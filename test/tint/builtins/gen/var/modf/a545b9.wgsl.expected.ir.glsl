SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};

void modf_a545b9() {
  f16vec2 arg_0 = f16vec2(-1.5hf);
  modf_result_vec2_f16 res = modf(arg_0);
}
void main() {
  modf_a545b9();
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'modf' : no matching overloaded function found 
ERROR: 0:14: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global 2-component vector of float16_t whole}'
ERROR: 0:14: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};

void modf_a545b9() {
  f16vec2 arg_0 = f16vec2(-1.5hf);
  modf_result_vec2_f16 res = modf(arg_0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_a545b9();
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'modf' : no matching overloaded function found 
ERROR: 0:12: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global 2-component vector of float16_t whole}'
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec2_f16 {
  f16vec2 fract;
  f16vec2 whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_a545b9() {
  f16vec2 arg_0 = f16vec2(-1.5hf);
  modf_result_vec2_f16 res = modf(arg_0);
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_a545b9();
  return tint_symbol;
}
void main() {
  gl_Position = vertex_main_inner().pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'modf' : no matching overloaded function found 
ERROR: 0:16: '=' :  cannot convert from ' const float' to ' temp structure{ global 2-component vector of float16_t fract,  global 2-component vector of float16_t whole}'
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
