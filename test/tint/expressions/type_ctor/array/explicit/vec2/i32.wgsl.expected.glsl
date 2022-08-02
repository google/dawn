#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec2 arr[2] = ivec2[2](ivec2(1), ivec2(2));
void f() {
  ivec2 v[2] = arr;
}

