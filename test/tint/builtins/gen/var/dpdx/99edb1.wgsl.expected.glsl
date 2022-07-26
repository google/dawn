#version 310 es
precision mediump float;

void dpdx_99edb1() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdx(arg_0);
}

void fragment_main() {
  dpdx_99edb1();
}

void main() {
  fragment_main();
  return;
}
