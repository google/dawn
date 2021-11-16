#version 310 es
precision mediump float;

void fwidthCoarse_1e59d9() {
  vec3 res = fwidth(vec3(0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  fwidthCoarse_1e59d9();
  return;
}
void main() {
  fragment_main();
}


