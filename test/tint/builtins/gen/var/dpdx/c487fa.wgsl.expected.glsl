#version 310 es
precision mediump float;

void dpdx_c487fa() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
}

void fragment_main() {
  dpdx_c487fa();
}

void main() {
  fragment_main();
  return;
}
