#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void main() {
  unused_entry_point();
}



struct A {
  int a;
};
struct B {
  int b;
};

B f(A a) {
  B tint_symbol = B(0);
  return tint_symbol;
}
