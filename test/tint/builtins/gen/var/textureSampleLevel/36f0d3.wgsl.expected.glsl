SKIP: FAILED

#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(0.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), 0.0f), float(arg_4), ivec2(0));
}

vec4 vertex_main() {
  textureSampleLevel_36f0d3();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'textureLodOffset' : no matching overloaded function found 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(0.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), 0.0f), float(arg_4), ivec2(0));
}

void fragment_main() {
  textureSampleLevel_36f0d3();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'textureLodOffset' : no matching overloaded function found 
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleLevel_36f0d3() {
  vec2 arg_2 = vec2(0.0f);
  int arg_3 = 1;
  uint arg_4 = 1u;
  float res = textureLodOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), 0.0f), float(arg_4), ivec2(0));
}

void compute_main() {
  textureSampleLevel_36f0d3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'textureLodOffset' : no matching overloaded function found 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



