#version 310 es
precision mediump float;

void f2() {
}

void f1() {
  f2();
}

void main() {
  f1();
  return;
}
