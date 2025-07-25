#version 310 es


struct S {
  ivec4 a[4];
};

int counter = 0;
int foo() {
  uint v = uint(counter);
  counter = int((v + uint(1)));
  return counter;
}
int bar() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(2)));
  return counter;
}
void v_2() {
  S x = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  uint v_3 = min(uint(foo()), 3u);
  int v_4 = bar();
  uint v_5 = uint(x.a[v_3][min(uint(v_4), 3u)]);
  int v_6 = int((v_5 + uint(5)));
  x.a[v_3][min(uint(v_4), 3u)] = v_6;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
