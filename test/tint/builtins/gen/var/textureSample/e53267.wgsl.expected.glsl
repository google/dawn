#version 310 es
precision mediump float;

uniform highp samplerCube arg_0_arg_1;

void textureSample_e53267() {
  vec3 arg_2 = vec3(1.0f);
  vec4 res = texture(arg_0_arg_1, arg_2);
}

void fragment_main() {
  textureSample_e53267();
}

void main() {
  fragment_main();
  return;
}
