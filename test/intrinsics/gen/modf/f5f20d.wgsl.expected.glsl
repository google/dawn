SKIP: FAILED

#version 310 es
precision mediump float;

struct modf_result_vec2 {
  vec2 fract;
  vec2 whole;
};
modf_result_vec2 tint_modf(vec2 param_0) {
  float2 whole;
  float2 fract = modf(param_0, whole);
  modf_result_vec2 result = {fract, whole};
  return result;
}


void modf_f5f20d() {
  modf_result_vec2 res = tint_modf(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  modf_f5f20d();
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
ERROR: 0:9: 'float2' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct modf_result_vec2 {
  vec2 fract;
  vec2 whole;
};
modf_result_vec2 tint_modf(vec2 param_0) {
  float2 whole;
  float2 fract = modf(param_0, whole);
  modf_result_vec2 result = {fract, whole};
  return result;
}


void modf_f5f20d() {
  modf_result_vec2 res = tint_modf(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  modf_f5f20d();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:9: 'float2' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

struct modf_result_vec2 {
  vec2 fract;
  vec2 whole;
};
modf_result_vec2 tint_modf(vec2 param_0) {
  float2 whole;
  float2 fract = modf(param_0, whole);
  modf_result_vec2 result = {fract, whole};
  return result;
}


void modf_f5f20d() {
  modf_result_vec2 res = tint_modf(vec2(0.0f, 0.0f));
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  modf_f5f20d();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:9: 'float2' : undeclared identifier 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



