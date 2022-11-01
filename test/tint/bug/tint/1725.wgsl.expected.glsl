#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  uint inner[];
} tint_symbol;

void tint_symbol_1(uint tint_symbol_2) {
  int tint_symbol_3 = 0;
  int tint_symbol_4 = 0;
  int tint_symbol_5 = 0;
  uint tint_symbol_6 = tint_symbol.inner[min(tint_symbol_2, (uint(tint_symbol.inner.length()) - 1u))];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_LocalInvocationIndex);
  return;
}
