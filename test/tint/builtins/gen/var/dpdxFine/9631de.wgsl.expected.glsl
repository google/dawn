#version 310 es
precision mediump float;

void dpdxFine_9631de() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxFine_9631de();
}

void main() {
  fragment_main();
  return;
}
