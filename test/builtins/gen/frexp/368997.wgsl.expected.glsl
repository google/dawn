SKIP: FAILED

#version 310 es

struct frexp_result_vec3 {
  vec3 sig;
  ivec3 exp;
};

frexp_result_vec3 tint_frexp(vec3 param_0) {
  float3 exp;
  float3 sig = frexp(param_0, exp);
  frexp_result_vec3 result = {sig, int3(exp)};
  return result;
}


void frexp_368997() {
  frexp_result_vec3 res = tint_frexp(vec3(0.0f, 0.0f, 0.0f));
}

vec4 vertex_main() {
  frexp_368997();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'float3' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct frexp_result_vec3 {
  vec3 sig;
  ivec3 exp;
};

frexp_result_vec3 tint_frexp(vec3 param_0) {
  float3 exp;
  float3 sig = frexp(param_0, exp);
  frexp_result_vec3 result = {sig, int3(exp)};
  return result;
}


void frexp_368997() {
  frexp_result_vec3 res = tint_frexp(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  frexp_368997();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'float3' : undeclared identifier 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct frexp_result_vec3 {
  vec3 sig;
  ivec3 exp;
};

frexp_result_vec3 tint_frexp(vec3 param_0) {
  float3 exp;
  float3 sig = frexp(param_0, exp);
  frexp_result_vec3 result = {sig, int3(exp)};
  return result;
}


void frexp_368997() {
  frexp_result_vec3 res = tint_frexp(vec3(0.0f, 0.0f, 0.0f));
}

void compute_main() {
  frexp_368997();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'float3' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



