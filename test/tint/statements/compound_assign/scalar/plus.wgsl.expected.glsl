#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int a;
};

layout(binding = 0, std430) buffer v_block_ssbo {
  S inner;
} v;

void foo() {
  v.inner.a = (v.inner.a + 2);
}

