#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdyCoarse_870a7e() {
  float res = dFdy(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdyCoarse_870a7e();
}
