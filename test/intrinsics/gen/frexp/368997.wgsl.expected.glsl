SKIP: FAILED

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

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  frexp_368997();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  vec4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = tint_symbol(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol outputs;
  outputs = vertex_main();
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
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

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  frexp_368997();
  return;
}
void main() {
  fragment_main();
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

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  frexp_368997();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:9: 'float3' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



