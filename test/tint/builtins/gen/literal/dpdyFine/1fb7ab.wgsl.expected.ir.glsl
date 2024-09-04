#version 310 es
precision highp float;
precision highp int;


vec3 prevent_dce;
vec3 dpdyFine_1fb7ab() {
  vec3 res = dFdy(vec3(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdyFine_1fb7ab();
}
