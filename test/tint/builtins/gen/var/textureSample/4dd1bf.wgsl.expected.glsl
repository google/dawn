SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0_arg_1;

void textureSample_4dd1bf() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec4 res = texture(arg_0_arg_1, vec4(arg_2, float(arg_3)));
}

void fragment_main() {
  textureSample_4dd1bf();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



