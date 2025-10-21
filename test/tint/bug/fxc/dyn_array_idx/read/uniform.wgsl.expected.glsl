#version 310 es


struct Result {
  int member_0;
};

layout(binding = 0, std140)
uniform ubo_block_1_ubo {
  uvec4 inner[5];
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_2 = v.inner[4u];
  uint v_3 = (16u * min(uint(int(v_2.x)), 3u));
  uvec4 v_4 = v.inner[(v_3 / 16u)];
  v_1.inner.member_0 = int(v_4[((v_3 % 16u) / 4u)]);
}
