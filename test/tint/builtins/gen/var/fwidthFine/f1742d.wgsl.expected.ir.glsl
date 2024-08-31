#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}
void main() {
  prevent_dce = fwidthFine_f1742d();
}
