#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSampleBias_9dbb51() {
  vec2 arg_2 = vec2(0.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  vec4 res = textureOffset(arg_0_arg_1, vec3(arg_2, float(arg_3)), ivec2(0), arg_4);
}

void fragment_main() {
  textureSampleBias_9dbb51();
}

void main() {
  fragment_main();
  return;
}
