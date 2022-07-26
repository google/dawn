#version 310 es
precision mediump float;

void fwidth_d2ab9a() {
  vec4 res = fwidth(vec4(1.0f));
}

void fragment_main() {
  fwidth_d2ab9a();
}

void main() {
  fragment_main();
  return;
}
