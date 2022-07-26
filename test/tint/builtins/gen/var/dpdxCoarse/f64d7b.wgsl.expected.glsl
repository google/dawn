#version 310 es
precision mediump float;

void dpdxCoarse_f64d7b() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxCoarse_f64d7b();
}

void main() {
  fragment_main();
  return;
}
