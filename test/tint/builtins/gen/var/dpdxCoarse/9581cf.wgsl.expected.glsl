#version 310 es
precision mediump float;

void dpdxCoarse_9581cf() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdx(arg_0);
}

void fragment_main() {
  dpdxCoarse_9581cf();
}

void main() {
  fragment_main();
  return;
}
