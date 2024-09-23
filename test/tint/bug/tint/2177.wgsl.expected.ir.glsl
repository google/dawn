#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  uint tint_symbol_1[];
} v;
uint f2() {
  return uint(v.tint_symbol_1.length());
}
uint f1() {
  return f2();
}
uint f0() {
  return f1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol_1[0] = f0();
}
