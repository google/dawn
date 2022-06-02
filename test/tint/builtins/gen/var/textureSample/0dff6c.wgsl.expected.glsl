#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSample_0dff6c() {
  vec2 arg_2 = vec2(0.0f);
  float res = textureOffset(arg_0_arg_1, vec3(arg_2, 0.0f), ivec2(0));
}

void fragment_main() {
  textureSample_0dff6c();
}

void main() {
  fragment_main();
  return;
}
