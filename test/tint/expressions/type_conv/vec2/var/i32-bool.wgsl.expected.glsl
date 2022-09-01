#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec2 u = ivec2(1);
void f() {
  bvec2 v = bvec2(u);
}

