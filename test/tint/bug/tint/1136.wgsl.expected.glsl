#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer Buffer_ssbo {
  uint data;
} tint_symbol;

void tint_symbol_1() {
  tint_symbol.data = (tint_symbol.data + 1u);
}

