#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_17e988() {
  vec2 arg_2 = vec2(0.0f);
  int arg_3 = 1;
  vec4 res = textureOffset(arg_0_arg_1, vec3(arg_2, float(arg_3)), ivec2(0));
}

void fragment_main() {
  textureSample_17e988();
}

void main() {
  fragment_main();
  return;
}
