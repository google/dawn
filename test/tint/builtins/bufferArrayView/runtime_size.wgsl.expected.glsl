#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_v_block_ssbo {
  uint inner[];
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
  uint v_4 = (uint(v_1.inner.length()) * 4u);
  uint v_5 = max(v_3, 16u);
  uint v_6 = 0u;
  uint v_7 = uaddCarry(0u, v_5, v_6);
  bool v_8 = (v_4 < mix(4294967295u, v_7, (v_6 == 0u)));
  uint v_9 = (((mix(0u, 0u, v_8) * 1u) + (min(0u, ((mix(v_5, 16u, v_8) / 16u) - 1u)) * 16u)) / 4u);
  uint v_10 = (v_9 + 1u);
  uint v_11 = (v_10 + 1u);
  v_2.inner = uvec4(v_1.inner[v_9], v_1.inner[v_10], v_1.inner[v_11], v_1.inner[(v_11 + 1u)]);
}
