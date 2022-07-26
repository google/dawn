#version 310 es
precision mediump float;

void dpdxFine_8c5069() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxFine_8c5069();
}

void main() {
  fragment_main();
  return;
}
