SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleCompare_7b5025() {
  float res = textureOffset(arg_0_arg_1, vec4(0.0f, 0.0f, float(1u), 1.0f), ivec2(0));
}

void fragment_main() {
  textureSampleCompare_7b5025();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



