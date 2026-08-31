#version 310 es


struct S {
  ivec4 a[4];
};

int counter = 0;
int foo() {
  counter = int((uint(counter) + 1u));
  return counter;
}
int bar() {
  counter = int((uint(counter) + 2u));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S x = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  uint v = min(uint(foo()), 3u);
  int v_1 = bar();
  int v_2 = int((uint(x.a[v][min(uint(v_1), 3u)]) + 5u));
  x.a[v][min(uint(v_1), 3u)] = v_2;
}
