#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int m;
  uint n[4];
};

uint f() {
  S a[2] = S[2](S(0, uint[4](0u, 0u, 0u, 0u)), S(0, uint[4](0u, 0u, 0u, 0u)));
  return a[1].n[1];
}

