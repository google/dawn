#version 310 es
precision mediump float;

void dpdyFine_df33aa() {
  vec2 res = dFdy(vec2(1.0f));
}

void fragment_main() {
  dpdyFine_df33aa();
}

void main() {
  fragment_main();
  return;
}
