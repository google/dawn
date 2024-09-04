#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdxFine_f92fb6() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdxFine_f92fb6();
}
