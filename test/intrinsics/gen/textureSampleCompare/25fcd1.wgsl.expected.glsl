SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSampleCompare_25fcd1() {
  float res = textureOffset(arg_0_arg_1, vec2(0.0f, 0.0f), 1.0f, ivec2(0, 0));
}

void fragment_main() {
  textureSampleCompare_25fcd1();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: 'textureOffset' : no matching overloaded function found 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



