#version 310 es
precision mediump float;

uniform highp sampler2D arg_0_arg_1;

void textureSample_6e64fb() {
  float arg_2 = 1.0f;
  vec4 res = texture(arg_0_arg_1, vec2(arg_2, 0.5f));
}

void fragment_main() {
  textureSample_6e64fb();
}

void main() {
  fragment_main();
  return;
}
