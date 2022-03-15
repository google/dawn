#version 310 es

struct Constants {
  uint zero;
};

layout(binding = 0) uniform Constants_1 {
  uint zero;
} constants;

struct Result {
  uint value;
};

layout(binding = 1, std430) buffer Result_1 {
  uint value;
} result;
struct TestData {
  int data[3];
};

layout(binding = 0, std430) buffer TestData_1 {
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
