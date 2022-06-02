#version 310 es
precision mediump float;

void dpdyFine_6eb673() {
  float arg_0 = 1.0f;
  float res = dFdy(arg_0);
}

void fragment_main() {
  dpdyFine_6eb673();
}

void main() {
  fragment_main();
  return;
}
