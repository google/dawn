#version 310 es
precision mediump float;

void dpdx_0763f7() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdx(arg_0);
}

void fragment_main() {
  dpdx_0763f7();
}

void main() {
  fragment_main();
  return;
}
