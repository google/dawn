SKIP: FAILED

#version 310 es
precision mediump float;

struct frexp_result_vec4 {
  vec4 sig;
  ivec4 exp;
};
frexp_result_vec4 tint_frexp(vec4 param_0) {
  float4 exp;
  float4 sig = frexp(param_0, exp);
  frexp_result_vec4 result = {sig, int4(exp)};
  return result;
}


void frexp_3c4f48() {
  frexp_result_vec4 res = tint_frexp(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  frexp_3c4f48();
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
ERROR: 0:9: 'float4' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct frexp_result_vec4 {
  vec4 sig;
  ivec4 exp;
};
frexp_result_vec4 tint_frexp(vec4 param_0) {
  float4 exp;
  float4 sig = frexp(param_0, exp);
  frexp_result_vec4 result = {sig, int4(exp)};
  return result;
}


void frexp_3c4f48() {
  frexp_result_vec4 res = tint_frexp(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  frexp_3c4f48();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:9: 'float4' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct frexp_result_vec4 {
  vec4 sig;
  ivec4 exp;
};
frexp_result_vec4 tint_frexp(vec4 param_0) {
  float4 exp;
  float4 sig = frexp(param_0, exp);
  frexp_result_vec4 result = {sig, int4(exp)};
  return result;
}


void frexp_3c4f48() {
  frexp_result_vec4 res = tint_frexp(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  frexp_3c4f48();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:9: 'float4' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



