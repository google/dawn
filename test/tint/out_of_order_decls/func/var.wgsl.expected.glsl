#version 310 es
precision highp float;

int a = 1;
void f() {
  int b = a;
}

void main() {
  f();
  return;
}
