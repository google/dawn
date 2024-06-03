#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  int inner[10];
} tint_symbol;

void foo() {
  int i = 0;
  while (true) {
    int x = tint_symbol.inner[i];
    {
      int x_1 = tint_symbol.inner[x];
      i = (i + x_1);
      if ((i > 10)) { break; }
    }
  }
  tint_symbol.inner[0] = i;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
  return;
}
