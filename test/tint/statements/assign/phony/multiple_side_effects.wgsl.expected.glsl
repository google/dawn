#version 310 es

int f(int a, int b, int c) {
  return ((a * b) + c);
}

void tint_symbol() {
  int tint_symbol_1 = f(1, 2, 3);
  int tint_symbol_2 = f(4, 5, 6);
  int tint_symbol_3 = f(8, 9, 10);
  int tint_symbol_4 = f(7, tint_symbol_3, 11);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
