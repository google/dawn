#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdy_7f8d84() {
  float res = dFdy(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdy_7f8d84();
}
