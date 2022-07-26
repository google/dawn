#version 310 es
precision mediump float;

void dpdyCoarse_ae1873() {
  vec3 res = dFdy(vec3(1.0f));
}

void fragment_main() {
  dpdyCoarse_ae1873();
}

void main() {
  fragment_main();
  return;
}
