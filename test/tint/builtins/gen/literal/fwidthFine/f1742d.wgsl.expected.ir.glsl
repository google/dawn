#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  return res;
}
void main() {
  prevent_dce = fwidthFine_f1742d();
}
