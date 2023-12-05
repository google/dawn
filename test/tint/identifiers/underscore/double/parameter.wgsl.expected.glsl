#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

void f(int tint_symbol) {
  int b = tint_symbol;
  s.inner = b;
}

void tint_symbol_1() {
  f(1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1();
  return;
}
