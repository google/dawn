#version 310 es
precision mediump float;

void dpdyCoarse_870a7e() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
}

void fragment_main() {
  dpdyCoarse_870a7e();
}

void main() {
  fragment_main();
  return;
}
