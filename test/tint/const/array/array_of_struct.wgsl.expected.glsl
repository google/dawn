#version 310 es


struct A {
  uvec2 b;
  uint c;
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  uint inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint q = 0u;
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  v.inner[v_2] = A[2](A(uvec2(1u, 2u), 3u), A(uvec2(4u, 5u), 6u))[min(q, 1u)].b.x;
}
