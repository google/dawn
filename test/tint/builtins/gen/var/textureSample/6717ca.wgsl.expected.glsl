#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_6717ca() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  vec4 res = texture(arg_0_arg_1, vec3(arg_2, float(arg_3)));
}

void fragment_main() {
  textureSample_6717ca();
}

void main() {
  fragment_main();
  return;
}
