#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int c(int x, int y, int z) {
  int a = (((1 + x) + y) + z);
  a = (a + 2);
  return a;
}

void b() {
  int b_1 = c(2, 3, 4);
  int tint_symbol = b_1;
  int tint_symbol_1 = c(3, 4, 5);
  b_1 = (tint_symbol + tint_symbol_1);
}

