#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSampleBias_1c707e() {
  vec4 res = texture(arg_0_arg_1, vec3(0.0f, 0.0f, float(1u)), 1.0f);
}

void fragment_main() {
  textureSampleBias_1c707e();
}

void main() {
  fragment_main();
  return;
}
