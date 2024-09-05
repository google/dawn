#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  int tint_symbol_1;
} v;
void f(int _A) {
  int B = _A;
  v.tint_symbol_1 = B;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
