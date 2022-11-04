#version 310 es
precision mediump float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

void textureSampleCompare_dd431d() {
  float res = texture(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 1.0f));
}

void fragment_main() {
  textureSampleCompare_dd431d();
}

void main() {
  fragment_main();
  return;
}
