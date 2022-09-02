#version 310 es

layout(binding = 1, std430) buffer data_block_ssbo {
  int inner[];
} data;

int foo() {
  return data.inner[0];
}

void tint_symbol() {
  foo();
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
