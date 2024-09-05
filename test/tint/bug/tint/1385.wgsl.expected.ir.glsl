#version 310 es

layout(binding = 1, std430)
buffer tint_symbol_2_1_ssbo {
  int tint_symbol_1[];
} v;
int foo() {
  return v.tint_symbol_1[0];
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  foo();
}
