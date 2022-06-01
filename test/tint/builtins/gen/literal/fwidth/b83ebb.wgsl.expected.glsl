#version 310 es
precision mediump float;

void fwidth_b83ebb() {
  vec2 res = fwidth(vec2(0.0f));
}

void fragment_main() {
  fwidth_b83ebb();
}

void main() {
  fragment_main();
  return;
}
