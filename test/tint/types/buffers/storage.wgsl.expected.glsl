#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_weights_block_ssbo {
  float inner[];
} v;
void main() {
  uint v_1 = min(0u, (uint(v.inner.length()) - 1u));
  float a = v.inner[v_1];
}
