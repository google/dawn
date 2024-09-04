#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdxFine_f401a2() {
  float res = dFdx(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdxFine_f401a2();
}
