#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  int inner;
} tint_symbol;

void tint_symbol_1() {
  int i = tint_symbol.inner;
  while (true) {
    {
      while (true) {
        if ((i > 5)) {
          i = (i * 2);
          break;
        } else {
          i = (i * 2);
          break;
        }
      }
      if ((i > 10)) { break; }
    }
  }
  tint_symbol.inner = i;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1();
  return;
}
