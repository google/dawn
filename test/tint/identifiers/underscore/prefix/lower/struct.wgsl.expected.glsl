#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct a {
  int b;
};

struct _a {
  int _b;
};

void f() {
  _a c = _a(0);
  int d = c._b;
}

