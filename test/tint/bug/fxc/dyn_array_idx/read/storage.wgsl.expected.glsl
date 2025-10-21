#version 310 es


struct Result {
  int member_0;
};

struct SSBO {
  int data[4];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(binding = 2, std430)
buffer ssbo_block_1_ssbo {
  SSBO inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_3 = v.inner[0u];
  uint v_4 = min(uint(int(v_3.x)), 3u);
  v_1.inner.member_0 = v_2.inner.data[v_4];
}
