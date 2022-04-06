#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int a = 1;
int a__ = 2;
void f() {
  int b = a;
  int b__ = a__;
}

