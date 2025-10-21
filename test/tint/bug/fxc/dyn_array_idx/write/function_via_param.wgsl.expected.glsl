#version 310 es


struct Result {
  int member_0;
};

struct S {
  int data[64];
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
void x(inout S p) {
  uvec4 v_2 = v.inner[0u];
  uint v_3 = min(uint(int(v_2.x)), 63u);
  p.data[v_3] = 1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S s = S(int[64](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  x(s);
  v_1.inner.member_0 = s.data[3u];
}
