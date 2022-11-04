#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSampleBias_9dbb51() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1)), ivec2(1), 1.0f);
}

void fragment_main() {
  textureSampleBias_9dbb51();
}

void main() {
  fragment_main();
  return;
}
