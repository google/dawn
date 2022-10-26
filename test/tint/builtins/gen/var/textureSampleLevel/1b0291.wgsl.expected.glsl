SKIP: FAILED

#version 310 es

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSampleLevel_1b0291() {
  vec3 arg_2 = vec3(0.0f);
  int arg_3 = 1;
  float res = textureLod(arg_0_arg_1, vec4(arg_2, 0.0f), float(arg_3));
}

vec4 vertex_main() {
  textureSampleLevel_1b0291();
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
ERROR: 0:8: 'textureLod' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSampleLevel_1b0291() {
  vec3 arg_2 = vec3(0.0f);
  int arg_3 = 1;
  float res = textureLod(arg_0_arg_1, vec4(arg_2, 0.0f), float(arg_3));
}

void fragment_main() {
  textureSampleLevel_1b0291();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:9: 'textureLod' : no matching overloaded function found 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSampleLevel_1b0291() {
  vec3 arg_2 = vec3(0.0f);
  int arg_3 = 1;
  float res = textureLod(arg_0_arg_1, vec4(arg_2, 0.0f), float(arg_3));
}

void compute_main() {
  textureSampleLevel_1b0291();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:8: 'textureLod' : no matching overloaded function found 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



