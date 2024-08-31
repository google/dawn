#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdx_e263de();
}
