#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  uint tint_symbol_1;
} v;
void tint_symbol() {
  v.tint_symbol_1 = (v.tint_symbol_1 + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
