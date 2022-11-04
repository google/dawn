#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSample_51b514() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = texture(arg_0_arg_1, arg_2);
}

void fragment_main() {
  textureSample_51b514();
}

void main() {
  fragment_main();
  return;
}
