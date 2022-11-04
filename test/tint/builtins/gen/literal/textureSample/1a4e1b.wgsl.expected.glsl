#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSample_1a4e1b() {
  float res = texture(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1u)), 0.0f));
}

void fragment_main() {
  textureSample_1a4e1b();
}

void main() {
  fragment_main();
  return;
}
