#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  ivec2 v2 = ivec2((1 + 2));
  ivec3 v3 = ivec3((1 + 2));
  ivec4 v4 = ivec4((1 + 2));
}

