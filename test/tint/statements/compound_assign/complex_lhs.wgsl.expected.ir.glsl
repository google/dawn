#version 310 es


struct S {
  ivec4 a[4];
};

int counter = 0;
int foo() {
  counter = (counter + 1);
  return counter;
}
int bar() {
  counter = (counter + 2);
  return counter;
}
void tint_symbol() {
  S x = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  int v = foo();
  int v_1 = bar();
  x.a[v][v_1] = (x.a[v][v_1] + 5);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
