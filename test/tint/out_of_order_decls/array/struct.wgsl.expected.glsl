#version 310 es
precision highp float;
precision highp int;

struct S {
  int m;
};

S A[4] = S[4](S(0), S(0), S(0), S(0));
void f() {
  S tint_symbol = S(1);
  A[0] = tint_symbol;
}

void main() {
  f();
  return;
}
