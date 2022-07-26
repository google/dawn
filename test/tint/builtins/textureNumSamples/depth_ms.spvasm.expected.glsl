SKIP: FAILED

#version 310 es

vec4 tint_symbol_1 = vec4(0.0f);
uniform highp sampler2DMS arg_0_1;
void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0_1);
  res = x_16;
  return;
}

void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2(vec4(0.0f));
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

vertex_main_out vertex_main() {
  vertex_main_1();
  vertex_main_out tint_symbol_3 = vertex_main_out(tint_symbol_1);
  return tint_symbol_3;
}

void main() {
  gl_PointSize = 1.0;
  vertex_main_out inner_result = vertex_main();
  gl_Position = inner_result.tint_symbol_1_1;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: 'textureSamples' : no matching overloaded function found 
ERROR: 0:7: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0_1;
void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0_1);
  res = x_16;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

void fragment_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

void fragment_main() {
  fragment_main_1();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: 'textureSamples' : no matching overloaded function found 
ERROR: 0:7: '=' :  cannot convert from ' const float' to ' temp mediump int'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

uniform highp sampler2DMS arg_0_1;
void textureNumSamples_a3c8a0() {
  int res = 0;
  int x_16 = textureSamples(arg_0_1);
  res = x_16;
  return;
}

struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

void compute_main_1() {
  textureNumSamples_a3c8a0();
  return;
}

void compute_main() {
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'textureSamples' : no matching overloaded function found 
ERROR: 0:6: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



