#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct T {
  uint k[2];
};

struct S {
  int m;
  T n[4];
};

uint f() {
  S a = S(0, T[4](T(uint[2](0u, 0u)), T(uint[2](0u, 0u)), T(uint[2](0u, 0u)), T(uint[2](0u, 0u))));
  return a.n[2].k[1];
}

