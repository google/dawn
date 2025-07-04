#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int _a = a;
  int b = a;
  int _b = _a;
  uint v_1 = uint(a);
  uint v_2 = uint(int((v_1 + uint(_a))));
  uint v_3 = uint(int((v_2 + uint(b))));
  v.inner = int((v_3 + uint(_b)));
}
