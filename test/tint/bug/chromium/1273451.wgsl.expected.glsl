#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
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

