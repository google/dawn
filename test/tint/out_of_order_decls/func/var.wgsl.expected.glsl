#version 310 es
precision highp float;
precision highp int;

int a = 1;
void f() {
  int b = a;
}

void main() {
  f();
  return;
}
