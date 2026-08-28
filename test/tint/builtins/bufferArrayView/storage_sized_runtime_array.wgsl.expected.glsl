#version 310 es

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  uint inner[32];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  min(uint(0), (8u - 1u));
  v_1.inner[0u] = 2u;
}
