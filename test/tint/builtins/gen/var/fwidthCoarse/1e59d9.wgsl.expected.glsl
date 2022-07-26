#version 310 es
precision mediump float;

void fwidthCoarse_1e59d9() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = fwidth(arg_0);
}

void fragment_main() {
  fwidthCoarse_1e59d9();
}

void main() {
  fragment_main();
  return;
}
