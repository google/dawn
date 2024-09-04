#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 dpdyCoarse_3e1ab4() {
  vec2 res = dFdy(vec2(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdyCoarse_3e1ab4();
}
