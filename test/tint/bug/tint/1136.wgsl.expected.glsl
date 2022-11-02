#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct Buffer {
  uint data;
};

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  Buffer inner;
} tint_symbol;

void tint_symbol_1() {
  tint_symbol.inner.data = (tint_symbol.inner.data + 1u);
}

