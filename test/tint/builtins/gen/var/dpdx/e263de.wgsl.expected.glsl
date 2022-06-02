#version 310 es
precision mediump float;

void dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
}

void fragment_main() {
  dpdx_e263de();
}

void main() {
  fragment_main();
  return;
}
