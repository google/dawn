#version 310 es


struct S {
  int a;
};

layout(binding = 0, std140)
uniform v_block_1_ubo {
  S inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = uint(v_1.inner.a);
  int u = int((v_2 + uint(1)));
}
