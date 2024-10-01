#version 310 es


struct Constants {
  uint zero;
};

struct Result {
  uint value;
};

struct TestData {
  int data[3];
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  Constants tint_symbol_1;
} v;
layout(binding = 1, std430)
buffer tint_symbol_4_1_ssbo {
  Result tint_symbol_3;
} v_1;
layout(binding = 0, std430)
buffer tint_symbol_6_1_ssbo {
  TestData tint_symbol_5;
} v_2;
int runTest() {
  uint v_3 = (0u + uint(v.tint_symbol_1.zero));
  return atomicOr(v_2.tint_symbol_5.data[v_3], 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.tint_symbol_3.value = uint(runTest());
}
