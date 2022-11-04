#version 310 es
precision mediump float;

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSampleCompare_63fb83() {
  vec3 arg_2 = vec3(1.0f);
  float arg_3 = 1.0f;
  float res = texture(arg_0_arg_1, vec4(arg_2, arg_3));
}

void fragment_main() {
  textureSampleCompare_63fb83();
}

void main() {
  fragment_main();
  return;
}
