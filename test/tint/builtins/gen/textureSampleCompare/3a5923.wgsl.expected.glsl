#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleCompare_3a5923() {
  float res = texture(arg_0_arg_1, vec3(0.0f, 0.0f, 1.0f));
}

void fragment_main() {
  textureSampleCompare_3a5923();
}

void main() {
  fragment_main();
  return;
}
