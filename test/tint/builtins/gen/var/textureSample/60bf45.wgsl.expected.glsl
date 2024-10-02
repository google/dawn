#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp sampler2DArrayShadow arg_0_arg_1;

float textureSample_60bf45() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float res = textureGradOffset(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), 0.0f), dFdx(arg_2), dFdy(arg_2), ivec2(1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_60bf45();
}

void main() {
  fragment_main();
  return;
}
