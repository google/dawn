#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleCompare_90ae56() {
  float res = texture(arg_0_arg_1, vec4(0.0f, 0.0f, float(1u), 1.0f));
}

void fragment_main() {
  textureSampleCompare_90ae56();
}

void main() {
  fragment_main();
  return;
}
