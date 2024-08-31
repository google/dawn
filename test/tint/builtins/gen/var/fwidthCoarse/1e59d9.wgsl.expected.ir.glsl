#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 fwidthCoarse_1e59d9() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = fwidth(arg_0);
  return res;
}
void main() {
  prevent_dce = fwidthCoarse_1e59d9();
}
