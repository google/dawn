#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 dpdy_699a05() {
  vec4 res = dFdy(vec4(1.0f));
  return res;
}
void main() {
  prevent_dce = dpdy_699a05();
}
