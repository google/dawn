#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer S_ssbo {
  ivec3 v;
  uint pad;
} U;

void f() {
  U.v = ivec3(1, 2, 3);
  U.v.x = 1;
  U.v.y = 2;
  U.v.z = 3;
}

