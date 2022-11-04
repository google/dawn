#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSampleBias_80e579() {
  vec4 res = texture(arg_0_arg_1, vec3(vec2(1.0f), float(1)), 1.0f);
}

void fragment_main() {
  textureSampleBias_80e579();
}

void main() {
  fragment_main();
  return;
}
