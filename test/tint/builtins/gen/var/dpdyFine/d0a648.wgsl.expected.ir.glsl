#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdyFine_d0a648() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdy(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdyFine_d0a648();
}
