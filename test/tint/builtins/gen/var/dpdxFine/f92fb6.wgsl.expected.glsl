#version 310 es
precision mediump float;

void dpdxFine_f92fb6() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxFine_f92fb6();
}

void main() {
  fragment_main();
  return;
}
