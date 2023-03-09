#version 310 es
precision highp float;

uniform highp sampler2D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_85c4ba() {
  vec4 res = textureOffset(arg_0_arg_1, vec2(1.0f), ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_85c4ba();
}

void main() {
  fragment_main();
  return;
}
