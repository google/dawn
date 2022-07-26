#version 310 es
precision mediump float;

void dpdxFine_8c5069() {
  vec4 res = dFdx(vec4(1.0f));
}

void fragment_main() {
  dpdxFine_8c5069();
}

void main() {
  fragment_main();
  return;
}
