#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2;
} v;
int a = 1;
int tint_symbol = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b = a;
  int tint_symbol_1 = tint_symbol;
  v.tint_symbol_2 = (b + tint_symbol_1);
}
