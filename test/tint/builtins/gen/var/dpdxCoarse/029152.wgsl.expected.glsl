#version 310 es
precision mediump float;

void dpdxCoarse_029152() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
}

void fragment_main() {
  dpdxCoarse_029152();
}

void main() {
  fragment_main();
  return;
}
