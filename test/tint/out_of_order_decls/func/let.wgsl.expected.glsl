#version 310 es
precision mediump float;

const int a = 1;
void f() {
  int b = a;
}

void main() {
  f();
  return;
}
