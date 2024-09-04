#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdyFine_6eb673() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdyFine_6eb673();
}
