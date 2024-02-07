#version 310 es
precision highp float;
precision highp int;

void tint_symbol() {
  int a[5] = int[5](0, 0, 0, 0, 0);
}

void main() {
  tint_symbol();
  return;
}
