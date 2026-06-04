#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  uvec4 inner[];
} v_1;
layout(binding = 1, std430)
buffer f_out_block_ssbo {
  uvec4 inner;
} v_2;
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void main() {
  uint size = 16u;
  uint v_3 = (tint_div_u32(size, 16u) * 16u);
  uint v_4 = max(v_3, 16u);
  bool v_5 = ((uint(v_1.inner.length()) * 16u) < v_4);
  uint v_6 = (((mix(0u, 0u, v_5) * 1u) + (min(uint(0), ((mix(v_4, 16u, v_5) / 16u) - 1u)) * 16u)) / 16u);
  v_2.inner = v_1.inner[v_6];
}
