#version 310 es

layout(binding = 0, std430) buffer tint_symbol_2_block_ssbo {
  uint inner;
} tint_symbol_2;

void tint_symbol_3() {
  tint_symbol_2.inner = 0u;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_3();
  return;
}
