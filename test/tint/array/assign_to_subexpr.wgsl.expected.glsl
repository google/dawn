#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  int inner;
} s;

struct S {
  int arr[4];
};

int foo() {
  int src[4] = int[4](0, 0, 0, 0);
  int dst[4] = int[4](0, 0, 0, 0);
  S dst_struct = S(int[4](0, 0, 0, 0));
  int dst_array[2][4] = int[2][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0));
  dst_struct.arr = src;
  dst_array[1] = src;
  dst = src;
  dst_struct.arr = src;
  dst_array[0] = src;
  return ((dst[0] + dst_struct.arr[0]) + dst_array[0][0]);
}

void tint_symbol() {
  s.inner = foo();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
