#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int A = 1;
  int _A = 2;
  int B = A;
  int _B = _A;
  uint v_1 = uint(A);
  uint v_2 = uint(int((v_1 + uint(_A))));
  uint v_3 = uint(int((v_2 + uint(B))));
  v.inner = int((v_3 + uint(_B)));
}
