#version 310 es

struct PixelLocal {
  uint a;
};
precision highp float;
precision highp int;


PixelLocal P;
void f0() {
  P.a = (P.a + 9u);
}
void f1() {
  f0();
  P.a = (P.a + 8u);
}
void f2() {
  P.a = (P.a + 7u);
  f1();
}
void main() {
  f2();
}
