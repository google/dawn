#version 310 es
precision mediump float;

void fwidthFine_ff6aa0() {
  vec2 res = fwidth(vec2(1.0f));
}

void fragment_main() {
  fwidthFine_ff6aa0();
}

void main() {
  fragment_main();
  return;
}
