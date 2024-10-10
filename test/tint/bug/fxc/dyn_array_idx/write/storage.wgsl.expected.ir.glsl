#version 310 es


struct UBO {
  int dynamic_idx;
};

struct Result {
  int tint_symbol;
};

struct SSBO {
  int data[4];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 2, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(binding = 1, std430)
buffer ssbo_block_1_ssbo {
  SSBO inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_3 = v.inner.dynamic_idx;
  v_2.inner.data[v_3] = 1;
  v_1.inner.tint_symbol = v_2.inner.data[3];
}
