#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdxFine_8c5069() {
  vec4 res = dFdx(vec4(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdxFine_8c5069();
}
