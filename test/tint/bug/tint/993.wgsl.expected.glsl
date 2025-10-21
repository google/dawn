#version 310 es


struct Result {
  uint value;
};

struct TestData {
  int data[3];
};

layout(binding = 0, std140)
uniform constants_block_1_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std430)
buffer result_block_1_ssbo {
  Result inner;
} v_1;
layout(binding = 2, std430)
buffer s_block_1_ssbo {
  TestData inner;
} v_2;
int runTest() {
  uvec4 v_3 = v.inner[0u];
  uint v_4 = min((0u + uint(v_3.x)), 2u);
  return atomicOr(v_2.inner.data[v_4], 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner.value = uint(runTest());
}
