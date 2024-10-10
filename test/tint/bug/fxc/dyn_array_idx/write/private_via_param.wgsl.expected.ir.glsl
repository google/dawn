#version 310 es


struct UBO {
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

struct S {
  int data[64];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
S s = S(int[64](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
void x(inout S p) {
  int v_2 = v.inner.dynamic_idx;
  p.data[v_2] = 1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  x(s);
  v_1.inner.tint_symbol = s.data[3];
}
