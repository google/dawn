#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 fwidth_5d1b39() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = fwidth(arg_0);
  return res;
}
void main() {
  prevent_dce = fwidth_5d1b39();
}
