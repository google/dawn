#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol[];
} v;
void n() {
  uint v_1 = uint(v.tint_symbol.length());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
