#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct T {
  float o;
  uint p;
};

struct S {
  int m;
  T n;
};

uint f() {
  S a = S(0, T(0.0f, 0u));
  return a.n.p;
}

