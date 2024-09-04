#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdyFine_6eb673() {
  float res = dFdy(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdyFine_6eb673();
}
