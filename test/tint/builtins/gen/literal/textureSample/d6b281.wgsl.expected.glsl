#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_d6b281() {
  vec4 res = texture(arg_0_arg_1, vec3(0.0f, 0.0f, float(1u)));
}

void fragment_main() {
  textureSample_d6b281();
}

void main() {
  fragment_main();
  return;
}
