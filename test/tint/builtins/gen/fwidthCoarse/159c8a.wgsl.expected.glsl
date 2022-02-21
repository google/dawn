#version 310 es
precision mediump float;

void fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
}

void fragment_main() {
  fwidthCoarse_159c8a();
}

void main() {
  fragment_main();
  return;
}
