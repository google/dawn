#version 310 es

layout(binding = 0, std430)
buffer arr_block_1_ssbo {
  uint inner[];
} v;
uint f2() {
  return uint(v.inner.length());
}
uint f1() {
  return f2();
}
uint f0() {
  return f1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(0u, (uint(v.inner.length()) - 1u));
  v.inner[v_1] = f0();
}
