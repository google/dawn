#version 310 es
precision mediump float;

void fwidthFine_523fdc() {
  vec3 res = fwidth(vec3(1.0f));
}

void fragment_main() {
  fwidthFine_523fdc();
}

void main() {
  fragment_main();
  return;
}
