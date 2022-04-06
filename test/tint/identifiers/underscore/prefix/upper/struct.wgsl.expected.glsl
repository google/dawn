#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct A {
  int B;
};

struct _A {
  int _B;
};

void f() {
  _A c = _A(0);
  int d = c._B;
}

