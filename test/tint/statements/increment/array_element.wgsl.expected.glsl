#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer a_block_ssbo {
  uint inner[];
} a;

void tint_symbol() {
  a.inner[1] = (a.inner[1] + 1u);
}

