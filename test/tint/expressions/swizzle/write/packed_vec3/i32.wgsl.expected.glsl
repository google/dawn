#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  ivec3 v;
  uint pad;
};

layout(binding = 0, std430) buffer U_block_ssbo {
  S inner;
} U;

void f() {
  U.inner.v = ivec3(1, 2, 3);
  U.inner.v.x = 1;
  U.inner.v.y = 2;
  U.inner.v.z = 3;
}

