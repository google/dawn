#version 310 es
precision highp float;

uniform highp sampler2DArray arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_17e988() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  vec4 res = textureOffset(arg_0_arg_1, vec3(arg_2, float(arg_3)), ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_17e988();
}

void main() {
  fragment_main();
  return;
}
