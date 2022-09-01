#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bvec2 u = bvec2(true);
void f() {
  ivec2 v = ivec2(u);
}

