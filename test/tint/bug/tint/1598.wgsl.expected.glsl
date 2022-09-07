#version 310 es
precision mediump float;

void tint_symbol() {
  int a[5] = int[5](0, 0, 0, 0, 0);
}

void main() {
  tint_symbol();
  return;
}
