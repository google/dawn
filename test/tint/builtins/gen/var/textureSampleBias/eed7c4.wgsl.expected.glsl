SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleBias_eed7c4() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  vec4 res = texture(arg_0_arg_1, vec4(arg_2, float(arg_3)), arg_4);
}

void fragment_main() {
  textureSampleBias_eed7c4();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



