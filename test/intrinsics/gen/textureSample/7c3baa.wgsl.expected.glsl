#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSample_7c3baa() {
  vec4 res = textureOffset(arg_0_arg_1, vec2(0.0f, 0.0f), ivec2(0, 0));
}

void fragment_main() {
  textureSample_7c3baa();
}

void main() {
  fragment_main();
  return;
}
