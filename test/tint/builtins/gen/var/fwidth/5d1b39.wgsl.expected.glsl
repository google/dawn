#version 310 es
precision mediump float;

void fwidth_5d1b39() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = fwidth(arg_0);
}

void fragment_main() {
  fwidth_5d1b39();
}

void main() {
  fragment_main();
  return;
}
