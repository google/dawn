#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSample_7e9ffd() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float res = texture(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), 0.0f));
}

void fragment_main() {
  textureSample_7e9ffd();
}

void main() {
  fragment_main();
  return;
}
