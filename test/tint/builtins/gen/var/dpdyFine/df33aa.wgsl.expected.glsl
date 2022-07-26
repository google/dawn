#version 310 es
precision mediump float;

void dpdyFine_df33aa() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdy(arg_0);
}

void fragment_main() {
  dpdyFine_df33aa();
}

void main() {
  fragment_main();
  return;
}
