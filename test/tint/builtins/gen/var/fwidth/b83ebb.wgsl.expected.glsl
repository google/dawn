#version 310 es
precision mediump float;

void fwidth_b83ebb() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = fwidth(arg_0);
}

void fragment_main() {
  fwidth_b83ebb();
}

void main() {
  fragment_main();
  return;
}
