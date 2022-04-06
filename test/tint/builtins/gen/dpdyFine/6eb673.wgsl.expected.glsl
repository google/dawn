#version 310 es
precision mediump float;

void dpdyFine_6eb673() {
  float res = dFdy(1.0f);
}

void fragment_main() {
  dpdyFine_6eb673();
}

void main() {
  fragment_main();
  return;
}
