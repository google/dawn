#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  ivec4 inner;
} tint_symbol;

void deref() {
  tint_symbol.inner = tint_symbol.inner.wzyx;
}

void no_deref() {
  tint_symbol.inner = tint_symbol.inner.wzyx;
}

void tint_symbol_1() {
  deref();
  no_deref();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1();
  return;
}
