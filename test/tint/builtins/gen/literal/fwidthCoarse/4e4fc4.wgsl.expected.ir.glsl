#version 310 es
precision highp float;
precision highp int;


vec4 prevent_dce;
vec4 fwidthCoarse_4e4fc4() {
  vec4 res = fwidth(vec4(1.0f));
  return res;
}
void main() {
  prevent_dce = fwidthCoarse_4e4fc4();
}
