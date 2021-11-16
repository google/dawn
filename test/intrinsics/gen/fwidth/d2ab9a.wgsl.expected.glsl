#version 310 es
precision mediump float;

void fwidth_d2ab9a() {
  vec4 res = fwidth(vec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void fragment_main() {
  fwidth_d2ab9a();
  return;
}
void main() {
  fragment_main();
}


