SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;
vec4 tint_symbol_1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0);;
  res = x_16;
  return;
}

void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

vertex_main_out vertex_main_inner() {
  vertex_main_1();
  vertex_main_out tint_symbol_4 = vertex_main_out(tint_symbol_1);
  return tint_symbol_4;
}

tint_symbol_3 vertex_main() {
  vertex_main_out inner_result = vertex_main_inner();
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.tint_symbol_1_1 = inner_result.tint_symbol_1_1;
  return wrapper_result;
}
void main() {
  tint_symbol_3 outputs;
  outputs = vertex_main();
  gl_Position = outputs.tint_symbol_1_1;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:9: 'textureSamples' : no matching overloaded function found 
ERROR: 0:9: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:9: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;

void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0);;
  res = x_16;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

void fragment_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:8: 'textureSamples' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp mediump int'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0;

void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0);;
  res = x_16;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  vec4 tint_symbol_1_1;
};

void compute_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  compute_main_1();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:8: 'textureSamples' : no matching overloaded function found 
ERROR: 0:8: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



