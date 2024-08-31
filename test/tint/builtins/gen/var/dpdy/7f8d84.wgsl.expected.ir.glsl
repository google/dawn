#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdy_7f8d84() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdy_7f8d84();
}
