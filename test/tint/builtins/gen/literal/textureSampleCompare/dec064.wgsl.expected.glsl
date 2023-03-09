#version 310 es
precision highp float;

uniform highp sampler2DShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompare_dec064() {
  float res = textureOffset(arg_0_arg_1, vec3(vec2(1.0f), 1.0f), ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleCompare_dec064();
}

void main() {
  fragment_main();
  return;
}
