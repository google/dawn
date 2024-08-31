#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
  return res;
}
void main() {
  prevent_dce = fwidthCoarse_159c8a();
}
