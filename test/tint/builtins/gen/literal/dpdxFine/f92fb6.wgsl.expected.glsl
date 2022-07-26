#version 310 es
precision mediump float;

void dpdxFine_f92fb6() {
  vec3 res = dFdx(vec3(1.0f));
}

void fragment_main() {
  dpdxFine_f92fb6();
}

void main() {
  fragment_main();
  return;
}
