#version 310 es
precision highp float;
precision highp int;


float prevent_dce;
float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}
void main() {
  prevent_dce = fwidth_df38ef();
}
