#version 310 es
precision mediump float;

void dpdxCoarse_9581cf() {
  vec2 res = dFdx(vec2(1.0f));
}

void fragment_main() {
  dpdxCoarse_9581cf();
}

void main() {
  fragment_main();
  return;
}
