#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 fwidth_d2ab9a() {
  vec4 res = fwidth(vec4(1.0f));
  return res;
}
void main() {
  prevent_dce = fwidth_d2ab9a();
}
