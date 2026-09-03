#version 310 es

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  uint inner[];
} v_1;
void foo() {
  bool v_2 = ((uint(v_1.inner.length()) * 4u) < 32u);
  v_1.inner[(((4u + (mix(0u, 0u, v_2) * 1u)) + (min(0u, ((mix(32u, 8u, v_2) / 8u) - 1u)) * 8u)) / 4u)] = 1077936128u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
}
