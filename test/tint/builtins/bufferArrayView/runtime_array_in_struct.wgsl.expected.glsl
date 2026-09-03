#version 310 es

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  uint inner[];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool v_2 = ((uint(v_1.inner.length()) * 4u) < 64u);
  v_1.inner[(((16u + (mix(0u, 0u, v_2) * 1u)) + (min(4u, (((mix(64u, 20u, v_2) - 16u) / 4u) - 1u)) * 4u)) / 4u)] = 4u;
}
