#version 310 es

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  uint inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(1u, (uint(v.inner.length()) - 1u));
  v.inner[v_1] = (v.inner[v_1] - 1u);
}
