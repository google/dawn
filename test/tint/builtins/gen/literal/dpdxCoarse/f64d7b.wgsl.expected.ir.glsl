#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdxCoarse_f64d7b() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_f64d7b();
}
