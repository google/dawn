#version 310 es
precision mediump float;

void fwidth_5d1b39() {
  vec3 res = fwidth(vec3(1.0f));
}

void fragment_main() {
  fwidth_5d1b39();
}

void main() {
  fragment_main();
  return;
}
