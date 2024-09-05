#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int A = 1;
int _A = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int B = A;
  int _B = _A;
  v.tint_symbol = (B + _B);
}
