#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSampleBias_80e579() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  vec4 res = texture(arg_0_arg_1, vec3(arg_2, float(arg_3)), arg_4);
}

void fragment_main() {
  textureSampleBias_80e579();
}

void main() {
  fragment_main();
  return;
}
