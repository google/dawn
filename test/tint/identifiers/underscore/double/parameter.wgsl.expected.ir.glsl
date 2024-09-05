#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2;
} v;
void f(int tint_symbol) {
  int b = tint_symbol;
  v.tint_symbol_2 = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
