SKIP: FAILED

#version 310 es

uniform highp sampler2DMS arg_0_1;
void textureNumSamples_2c6f14() {
  int res = textureSamples(arg_0_1);
}

vec4 vertex_main() {
  textureNumSamples_2c6f14();
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
ERROR: 0:5: 'textureSamples' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp sampler2DMS arg_0_1;
void textureNumSamples_2c6f14() {
  int res = textureSamples(arg_0_1);
}

void fragment_main() {
  textureNumSamples_2c6f14();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'textureSamples' : no matching overloaded function found 
ERROR: 0:6: '=' :  cannot convert from ' const float' to ' temp mediump int'
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

uniform highp sampler2DMS arg_0_1;
void textureNumSamples_2c6f14() {
  int res = textureSamples(arg_0_1);
}

void compute_main() {
  textureNumSamples_2c6f14();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'textureSamples' : no matching overloaded function found 
ERROR: 0:5: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



