#version 310 es
precision mediump float;

void dpdxCoarse_c28641() {
  vec4 res = dFdx(vec4(1.0f));
}

void fragment_main() {
  dpdxCoarse_c28641();
}

void main() {
  fragment_main();
  return;
}
