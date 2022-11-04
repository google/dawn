#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSample_38bbb9() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 0.0f));
}

void fragment_main() {
  textureSample_38bbb9();
}

void main() {
  fragment_main();
  return;
}
