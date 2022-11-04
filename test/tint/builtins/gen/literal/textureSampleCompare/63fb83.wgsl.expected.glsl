#version 310 es
precision mediump float;

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSampleCompare_63fb83() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), 1.0f));
}

void fragment_main() {
  textureSampleCompare_63fb83();
}

void main() {
  fragment_main();
  return;
}
