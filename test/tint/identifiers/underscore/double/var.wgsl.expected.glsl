#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

int a = 1;
int tint_symbol = 2;
void f() {
  int b = a;
  int tint_symbol_1 = tint_symbol;
  s.inner = (b + tint_symbol_1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
