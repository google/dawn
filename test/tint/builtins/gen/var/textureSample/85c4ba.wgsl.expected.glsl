#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSample_85c4ba() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec2(1));
}

void fragment_main() {
  textureSample_85c4ba();
}

void main() {
  fragment_main();
  return;
}
