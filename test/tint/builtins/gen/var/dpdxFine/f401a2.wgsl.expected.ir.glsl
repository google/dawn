#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdxFine_f401a2();
}
