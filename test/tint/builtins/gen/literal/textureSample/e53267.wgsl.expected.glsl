#version 310 es
precision mediump float;

uniform highp samplerCube arg_0_arg_1;

void textureSample_e53267() {
  vec4 res = texture(arg_0_arg_1, vec3(1.0f));
}

void fragment_main() {
  textureSample_e53267();
}

void main() {
  fragment_main();
  return;
}
