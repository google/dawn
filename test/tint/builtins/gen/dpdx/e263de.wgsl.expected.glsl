#version 310 es
precision mediump float;

void dpdx_e263de() {
  float res = dFdx(1.0f);
}

void fragment_main() {
  dpdx_e263de();
}

void main() {
  fragment_main();
  return;
}
