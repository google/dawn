#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 fwidthCoarse_e653f7() {
  vec2 res = fwidth(vec2(1.0f));
  return res;
}
void main() {
  prevent_dce = fwidthCoarse_e653f7();
}
