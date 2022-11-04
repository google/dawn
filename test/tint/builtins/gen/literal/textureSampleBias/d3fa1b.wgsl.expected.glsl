#version 310 es
precision mediump float;

uniform highp sampler3D arg_0_arg_1;

void textureSampleBias_d3fa1b() {
  vec4 res = texture(arg_0_arg_1, vec3(1.0f), 1.0f);
}

void fragment_main() {
  textureSampleBias_d3fa1b();
}

void main() {
  fragment_main();
  return;
}
