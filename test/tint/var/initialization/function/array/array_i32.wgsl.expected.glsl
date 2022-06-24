#version 310 es

void tint_symbol() {
  int zero[2][3] = int[2][3](int[3](0, 0, 0), int[3](0, 0, 0));
  int tint_symbol_1[3] = int[3](1, 2, 3);
  int tint_symbol_2[3] = int[3](4, 5, 6);
  int init[2][3] = int[2][3](tint_symbol_1, tint_symbol_2);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
