#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdyCoarse_870a7e() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdyCoarse_870a7e();
}
