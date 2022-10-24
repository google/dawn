SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleBias_c6953d() {
  vec4 res = texture(arg_0_arg_1, vec4(0.0f, 0.0f, 0.0f, float(1u)), 1.0f);
}

void fragment_main() {
  textureSampleBias_c6953d();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



