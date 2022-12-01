#version 310 es

layout(binding = 0, std430) buffer S_block_ssbo {
  int inner;
} S;

int func_S() {
  return S.inner;
}

void tint_symbol() {
  int r = func_S();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
