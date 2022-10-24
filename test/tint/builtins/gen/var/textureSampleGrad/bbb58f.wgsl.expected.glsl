SKIP: FAILED

#version 310 es

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleGrad_bbb58f() {
  vec3 arg_2 = vec3(0.0f);
  uint arg_3 = 1u;
  vec3 arg_4 = vec3(0.0f);
  vec3 arg_5 = vec3(0.0f);
  vec4 res = textureGrad(arg_0_arg_1, vec4(arg_2, float(arg_3)), arg_4, arg_5);
}

vec4 vertex_main() {
  textureSampleGrad_bbb58f();
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
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleGrad_bbb58f() {
  vec3 arg_2 = vec3(0.0f);
  uint arg_3 = 1u;
  vec3 arg_4 = vec3(0.0f);
  vec3 arg_5 = vec3(0.0f);
  vec4 res = textureGrad(arg_0_arg_1, vec4(arg_2, float(arg_3)), arg_4, arg_5);
}

void fragment_main() {
  textureSampleGrad_bbb58f();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleGrad_bbb58f() {
  vec3 arg_2 = vec3(0.0f);
  uint arg_3 = 1u;
  vec3 arg_4 = vec3(0.0f);
  vec3 arg_5 = vec3(0.0f);
  vec4 res = textureGrad(arg_0_arg_1, vec4(arg_2, float(arg_3)), arg_4, arg_5);
}

void compute_main() {
  textureSampleGrad_bbb58f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



