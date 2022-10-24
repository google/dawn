SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

void textureSample_7fd8cb() {
  float res = texture(arg_0_arg_1, vec4(0.0f, 0.0f, 0.0f, float(1u)), 0.0f);
}

void fragment_main() {
  textureSample_7fd8cb();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArrayShadow' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



