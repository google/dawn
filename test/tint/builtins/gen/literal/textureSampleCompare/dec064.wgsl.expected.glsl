#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleCompare_dec064() {
  float res = textureOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 1.0f), ivec2(0));
}

void fragment_main() {
  textureSampleCompare_dec064();
}

void main() {
  fragment_main();
  return;
}
