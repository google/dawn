#version 310 es
precision mediump float;

void fwidthCoarse_e653f7() {
  vec2 res = fwidth(vec2(1.0f));
}

void fragment_main() {
  fwidthCoarse_e653f7();
}

void main() {
  fragment_main();
  return;
}
