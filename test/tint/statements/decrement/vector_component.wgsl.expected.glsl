#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer a_block_ssbo {
  uvec4 inner;
} a;

void tint_symbol() {
  int tint_symbol_2 = 1;
  a.inner[tint_symbol_2] = (a.inner[tint_symbol_2] - 1u);
  a.inner.z = (a.inner.z - 1u);
}

