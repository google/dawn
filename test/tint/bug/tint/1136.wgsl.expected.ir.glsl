#version 310 es


struct Buffer {
  uint data;
};

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  Buffer tint_symbol_2;
} v;
void tint_symbol_1() {
  v.tint_symbol_2.data = (v.tint_symbol_2.data + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
