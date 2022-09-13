#version 310 es

layout(binding = 0, std140) uniform Constants_ubo {
  uint zero;
  uint pad;
  uint pad_1;
  uint pad_2;
} constants;

layout(binding = 1, std430) buffer Result_ssbo {
  uint value;
} result;

layout(binding = 0, std430) buffer TestData_ssbo {
  int data[3];
} s;

int runTest() {
  return atomicOr(s.data[(0u + uint(constants.zero))], 0);
}

void tint_symbol() {
  int tint_symbol_1 = runTest();
  uint tint_symbol_2 = uint(tint_symbol_1);
  result.value = tint_symbol_2;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
