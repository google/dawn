#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  uint inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint q = 0u;
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  v.inner[v_2] = uvec2[2](uvec2(1u, 2u), uvec2(3u, 4u))[min(q, 1u)].x;
}
