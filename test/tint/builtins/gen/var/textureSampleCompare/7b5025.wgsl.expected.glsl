SKIP: FAILED

#version 310 es
precision highp float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompare_7b5025() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float res = textureOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), arg_4), ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleCompare_7b5025();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'sampler' : TextureOffset does not support sampler2DArrayShadow :  ES Profile
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



