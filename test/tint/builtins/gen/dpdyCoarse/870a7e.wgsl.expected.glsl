#version 310 es
precision mediump float;

void dpdyCoarse_870a7e() {
  float res = dFdy(1.0f);
}

void fragment_main() {
  dpdyCoarse_870a7e();
}

void main() {
  fragment_main();
  return;
}
