#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int f(int i) {
  int a[8] = int[8](1, 2, 3, 4, 5, 6, 7, 8);
  return a[i];
}

