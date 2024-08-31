#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdx_c487fa() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdx_c487fa();
}
