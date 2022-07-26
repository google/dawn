#version 310 es
precision mediump float;

void dpdyCoarse_445d24() {
  vec4 res = dFdy(vec4(1.0f));
}

void fragment_main() {
  dpdyCoarse_445d24();
}

void main() {
  fragment_main();
  return;
}
