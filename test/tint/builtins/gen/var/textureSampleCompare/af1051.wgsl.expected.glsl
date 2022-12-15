SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleCompare_af1051() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  float res = textureOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), arg_4), ivec2(1));
}

void fragment_main() {
  textureSampleCompare_af1051();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:10: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



