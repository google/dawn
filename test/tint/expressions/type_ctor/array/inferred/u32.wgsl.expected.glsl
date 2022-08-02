#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint arr[2] = uint[2](1u, 2u);
void f() {
  uint v[2] = arr;
}

