#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int c = 0;
  int d = 0;
  int v_1 = d;
  uint v_2 = uint(c);
  v.inner = int((v_2 + uint(v_1)));
}
