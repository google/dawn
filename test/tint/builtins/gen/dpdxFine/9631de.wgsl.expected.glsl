#version 310 es
precision mediump float;

void dpdxFine_9631de() {
  vec2 res = dFdx(vec2(0.0f, 0.0f));
}

void fragment_main() {
  dpdxFine_9631de();
}

void main() {
  fragment_main();
  return;
}
