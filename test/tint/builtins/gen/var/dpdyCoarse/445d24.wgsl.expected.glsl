#version 310 es
precision mediump float;

void dpdyCoarse_445d24() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyCoarse_445d24();
}

void main() {
  fragment_main();
  return;
}
