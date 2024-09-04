#version 310 es
precision highp float;
precision highp int;


vec2 prevent_dce;
vec2 dpdyFine_df33aa() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdy(arg_0);
  return res;
}
void main() {
  prevent_dce = dpdyFine_df33aa();
}
