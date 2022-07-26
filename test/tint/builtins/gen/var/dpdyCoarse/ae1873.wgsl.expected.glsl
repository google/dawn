#version 310 es
precision mediump float;

void dpdyCoarse_ae1873() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyCoarse_ae1873();
}

void main() {
  fragment_main();
  return;
}
