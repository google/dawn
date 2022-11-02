#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  vec3 v;
  uint pad;
};

layout(binding = 0, std430) buffer U_block_ssbo {
  S inner;
} U;

void f() {
  U.inner.v = vec3(1.0f, 2.0f, 3.0f);
  U.inner.v.x = 1.0f;
  U.inner.v.y = 2.0f;
  U.inner.v.z = 3.0f;
}

