#version 310 es

int s;
int a = 1;
int tint_symbol = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b = a;
  int tint_symbol_1 = tint_symbol;
  s = (b + tint_symbol_1);
}
