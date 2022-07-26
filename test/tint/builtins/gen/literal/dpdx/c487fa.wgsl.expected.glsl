#version 310 es
precision mediump float;

void dpdx_c487fa() {
  vec4 res = dFdx(vec4(1.0f));
}

void fragment_main() {
  dpdx_c487fa();
}

void main() {
  fragment_main();
  return;
}
