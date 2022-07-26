#version 310 es
precision mediump float;

void dpdxCoarse_f64d7b() {
  vec3 res = dFdx(vec3(1.0f));
}

void fragment_main() {
  dpdxCoarse_f64d7b();
}

void main() {
  fragment_main();
  return;
}
