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
void main() {
  uint offset = 16u;
  uint v_3 = (offset & 4294967280u);
  uint v_4 = (uint(v_1.inner.length()) * 16u);
  uint v_5 = 0u;
  uint v_6 = uaddCarry(16u, v_3, v_5);
  bool v_7 = (v_4 < mix(4294967295u, v_6, (v_5 == 0u)));
  uint v_8 = (((mix(v_3, 0u, v_7) * 1u) + (min(uint(0), ((mix(16u, 16u, v_7) / 16u) - 1u)) * 16u)) / 16u);
  v_2.inner = v_1.inner[v_8];
}
