#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdxCoarse_c28641() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_c28641();
}
