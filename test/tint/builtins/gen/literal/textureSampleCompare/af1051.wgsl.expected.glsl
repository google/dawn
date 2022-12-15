SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleCompare_af1051() {
  float res = textureOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 1.0f), ivec2(1));
}

void fragment_main() {
  textureSampleCompare_af1051();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:7: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



