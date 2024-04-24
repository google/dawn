#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  uint inner;
} tint_symbol;

uint foo() {
  uint tint_symbol_4[4] = uint[4](0u, 1u, 2u, 4u);
  return tint_symbol_4[tint_symbol.inner];
}

void tint_symbol_1() {
  uint tint_symbol_5[4] = uint[4](0u, 1u, 2u, 4u);
  uint v = tint_symbol_5[tint_symbol.inner];
  uint tint_symbol_2 = v;
  uint tint_symbol_3 = foo();
  tint_symbol.inner = (tint_symbol_2 + tint_symbol_3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1();
  return;
}
