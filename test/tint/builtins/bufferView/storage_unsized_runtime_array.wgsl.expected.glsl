#version 310 es

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  uint inner[];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = (mix(0u, 0u, ((uint(v_1.inner.length()) * 4u) < 4u)) * 1u);
  uint v_3 = ((v_2 + (min(4u, ((uint(v_1.inner.length()) - (v_2 / 4u)) - 1u)) * 4u)) / 4u);
  v_1.inner[v_3] = 4u;
}
