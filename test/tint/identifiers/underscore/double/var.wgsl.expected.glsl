#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
int a = 1;
int v_1 = 2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b = a;
  int v_2 = v_1;
  int v_3 = v_2;
  uint v_4 = uint(b);
  v.inner = int((v_4 + uint(v_3)));
}
