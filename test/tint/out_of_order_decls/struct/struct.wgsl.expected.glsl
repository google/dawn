#version 310 es
precision mediump float;

struct S2 {
  int m;
};

struct S1 {
  S2 m;
};

void f() {
  S1 v = S1(S2(0));
}

void main() {
  f();
  return;
}
