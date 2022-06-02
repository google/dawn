#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleCompare_dec064() {
  vec2 arg_2 = vec2(0.0f);
  float arg_3 = 1.0f;
  float res = textureOffset(arg_0_arg_1, vec3(arg_2, arg_3), ivec2(0));
}

void fragment_main() {
  textureSampleCompare_dec064();
}

void main() {
  fragment_main();
  return;
}
