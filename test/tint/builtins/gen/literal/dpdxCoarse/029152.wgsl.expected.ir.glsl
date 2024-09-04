#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float dpdxCoarse_029152() {
  float res = dFdx(1.0f);
  return res;
}
void main() {
  prevent_dce = dpdxCoarse_029152();
}
