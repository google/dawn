#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdx_e263de() {
  float res = dFdx(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdx_e263de();
}
