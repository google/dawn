#version 310 es

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  uint inner[];
} v_1;
void foo() {
  uint v_2 = ((4u + (mix(0u, 0u, ((uint(v_1.inner.length()) * 4u) < 8u)) * 1u)) / 4u);
  v_1.inner[v_2] = 1077936128u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
}
