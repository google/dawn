#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  uint inner[2];
} tint_symbol;

void tint_symbol_1() {
  if (false) {
    tint_symbol.inner[0] = 1u;
  }
  if (false) {
    tint_symbol.inner[1] = 1u;
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1();
  return;
}
