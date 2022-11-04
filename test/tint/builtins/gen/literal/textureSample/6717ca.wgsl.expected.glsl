#version 310 es
precision mediump float;

uniform highp sampler2DArray arg_0_arg_1;

void textureSample_6717ca() {
  vec4 res = texture(arg_0_arg_1, vec3(vec2(1.0f), float(1)));
}

void fragment_main() {
  textureSample_6717ca();
}

void main() {
  fragment_main();
  return;
}
