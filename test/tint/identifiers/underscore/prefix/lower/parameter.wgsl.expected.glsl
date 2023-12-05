#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

void f(int _a) {
  int b = _a;
  s.inner = b;
}

void tint_symbol() {
  f(1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
