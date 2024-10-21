#version 310 es

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  uint inner[];
} v;
void tint_symbol() {
  v.inner[1] = (v.inner[1] - 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
