#version 310 es

struct S {
  int m;
};
precision highp float;
precision highp int;


S A[4] = S[4](S(0), S(0), S(0), S(0));
void main() {
  A[0] = S(1);
}
