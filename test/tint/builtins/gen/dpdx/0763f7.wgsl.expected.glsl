#version 310 es
precision mediump float;

void dpdx_0763f7() {
  vec3 res = dFdx(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  dpdx_0763f7();
}

void main() {
  fragment_main();
  return;
}
