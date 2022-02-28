#version 310 es
precision mediump float;

void dpdxFine_f401a2() {
  float res = dFdx(1.0f);
}

void fragment_main() {
  dpdxFine_f401a2();
}

void main() {
  fragment_main();
  return;
}
