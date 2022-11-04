#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSampleBias_a161cf() {
  vec2 arg_2 = vec2(1.0f);
  float arg_3 = 1.0f;
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec2(1), arg_3);
}

void fragment_main() {
  textureSampleBias_a161cf();
}

void main() {
  fragment_main();
  return;
}
