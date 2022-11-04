#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSampleBias_6a9113() {
  vec4 res = texture(arg_0_arg_1, vec2(1.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_6a9113();
}

void main() {
  fragment_main();
  return;
}
