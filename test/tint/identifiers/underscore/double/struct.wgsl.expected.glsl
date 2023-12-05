#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

struct a {
  int b;
};

struct tint_symbol {
  int tint_symbol_1;
};

void f() {
  tint_symbol c = tint_symbol(0);
  int d = c.tint_symbol_1;
  s.inner = (c.tint_symbol_1 + d);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
