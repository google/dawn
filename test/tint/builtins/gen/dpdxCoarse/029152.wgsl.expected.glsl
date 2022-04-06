#version 310 es
precision mediump float;

void dpdxCoarse_029152() {
  float res = dFdx(1.0f);
}

void fragment_main() {
  dpdxCoarse_029152();
}

void main() {
  fragment_main();
  return;
}
