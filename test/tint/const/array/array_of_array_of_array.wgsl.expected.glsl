#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  uint inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint q = 0u;
  uint v_1 = min(0u, (uint(v.inner.length()) - 1u));
  v.inner[v_1] = uint[2][2][2](uint[2][2](uint[2](0u, 1u), uint[2](2u, 3u)), uint[2][2](uint[2](4u, 5u), uint[2](6u, 7u)))[min(q, 1u)][min(q, 1u)][min(q, 1u)];
}
