#version 310 es

struct str {
  int arr[4];
};

layout(binding = 0, std430) buffer S_block_ssbo {
  str inner;
} S;

void func_S_inner_arr() {
  int tint_symbol_1[4] = int[4](0, 0, 0, 0);
  S.inner.arr = tint_symbol_1;
}

void tint_symbol() {
  func_S_inner_arr();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
