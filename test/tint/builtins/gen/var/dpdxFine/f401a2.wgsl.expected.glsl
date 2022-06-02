#version 310 es
precision mediump float;

void dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
}

void fragment_main() {
  dpdxFine_f401a2();
}

void main() {
  fragment_main();
  return;
}
