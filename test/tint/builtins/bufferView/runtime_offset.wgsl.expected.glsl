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
  uint v_3 = (uint(v_1.inner.length()) * 16u);
  uint v_4 = 0u;
  uint v_5 = uaddCarry(16u, (offset & 4294967280u), v_4);
  uint v_6 = ((mix((offset & 4294967280u), 0u, (v_3 < mix(4294967295u, v_5, (v_4 == 0u)))) * 1u) / 16u);
  v_2.inner = v_1.inner[v_6];
}
