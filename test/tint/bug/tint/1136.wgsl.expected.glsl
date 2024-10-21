#version 310 es


struct Buffer {
  uint data;
};

layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
  Buffer inner;
} v;
void tint_symbol_1() {
  v.inner.data = (v.inner.data + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
