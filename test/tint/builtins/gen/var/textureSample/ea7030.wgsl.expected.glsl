#version 310 es
precision mediump float;

uniform highp samplerCubeShadow arg_0_arg_1;

void textureSample_ea7030() {
  vec3 arg_2 = vec3(1.0f);
  float res = texture(arg_0_arg_1, vec4(arg_2, 0.0f));
}

void fragment_main() {
  textureSample_ea7030();
}

void main() {
  fragment_main();
  return;
}
