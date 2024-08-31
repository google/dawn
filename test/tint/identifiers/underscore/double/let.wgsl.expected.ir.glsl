#version 310 es

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int tint_symbol = a;
  int b = a;
  int tint_symbol_1 = tint_symbol;
  s = (((a + tint_symbol) + b) + tint_symbol_1);
}
