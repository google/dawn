#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdx_c487fa() {
  vec4 res = dFdx(vec4(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdx_c487fa();
}
