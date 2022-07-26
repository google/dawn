#version 310 es
precision mediump float;

void dpdyCoarse_3e1ab4() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyCoarse_3e1ab4();
}

void main() {
  fragment_main();
  return;
}
