#version 310 es
precision highp float;
precision highp int;

struct S {
  int m;
};

void f() {
  S v = S(0);
}

void main() {
  f();
  return;
}
