#version 310 es

layout(binding = 0, std430)
buffer data_block_1_ssbo {
  int inner[];
} v;
int foo() {
  uint v_1 = min(0u, (uint(v.inner.length()) - 1u));
  return v.inner[v_1];
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  foo();
}
