#version 310 es
precision mediump float;

void dpdyCoarse_3e1ab4() {
  vec2 res = dFdy(vec2(1.0f));
}

void fragment_main() {
  dpdyCoarse_3e1ab4();
}

void main() {
  fragment_main();
  return;
}
