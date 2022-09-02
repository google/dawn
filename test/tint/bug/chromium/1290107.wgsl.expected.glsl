#version 310 es

struct S {
  float f;
};

layout(binding = 0, std430) buffer arr_block_ssbo {
  S inner[];
} arr;

void tint_symbol() {
  uint len = uint(arr.inner.length());
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
