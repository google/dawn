#version 310 es

struct str {
  int arr[4];
};

layout(binding = 0, std430) buffer S_block_ssbo {
  str inner;
} S;

int[4] func_S_arr() {
  return S.inner.arr;
}

void tint_symbol() {
  int r[4] = func_S_arr();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
