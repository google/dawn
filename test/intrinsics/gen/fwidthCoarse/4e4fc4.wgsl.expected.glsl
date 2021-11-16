#version 310 es
precision mediump float;

void fwidthCoarse_4e4fc4() {
  vec4 res = fwidth(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  fwidthCoarse_4e4fc4();
  return;
}
void main() {
  fragment_main();
}


