#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 dpdy_a8b56e() {
  vec2 res = dFdy(vec2(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdy_a8b56e();
}
