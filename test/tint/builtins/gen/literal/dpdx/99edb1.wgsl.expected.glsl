#version 310 es
precision mediump float;

void dpdx_99edb1() {
  vec2 res = dFdx(vec2(1.0f));
}

void fragment_main() {
  dpdx_99edb1();
}

void main() {
  fragment_main();
  return;
}
