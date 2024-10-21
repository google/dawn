#version 310 es

layout(binding = 1, std430)
buffer data_block_1_ssbo {
  int inner[];
} v;
int foo() {
  return v.inner[0];
}
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  foo();
}
