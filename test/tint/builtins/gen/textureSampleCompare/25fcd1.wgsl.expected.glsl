#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleCompare_25fcd1() {
  float res = textureOffset(arg_0_arg_1, vec3(0.0f, 0.0f, 1.0f), ivec2(0, 0));
}

void fragment_main() {
  textureSampleCompare_25fcd1();
}

void main() {
  fragment_main();
  return;
}
