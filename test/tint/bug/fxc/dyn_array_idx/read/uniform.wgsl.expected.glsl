#version 310 es


struct UBO {
  ivec4 data[4];
  int dynamic_idx;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
};

struct Result {
  int tint_symbol;
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  UBO inner;
} v;
layout(binding = 2, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_2 = v.inner.dynamic_idx;
  v_1.inner.tint_symbol = v.inner.data[v_2].x;
}
