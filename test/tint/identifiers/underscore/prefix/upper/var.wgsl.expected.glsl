#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int A = 1;
int _A = 2;
void f() {
  int B = A;
  int _B = _A;
}

