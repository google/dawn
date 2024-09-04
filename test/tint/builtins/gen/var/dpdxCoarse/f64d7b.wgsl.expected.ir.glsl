#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdxCoarse_f64d7b() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = dFdx(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_f64d7b();
}
