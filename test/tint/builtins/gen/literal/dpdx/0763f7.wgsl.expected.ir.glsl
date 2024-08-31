#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdx_0763f7() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdx_0763f7();
}
