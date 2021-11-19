#version 310 es
precision mediump float;


layout (binding = 0) uniform Constants_1 {
  uint zero;
} constants;

layout (binding = 1) buffer Result_1 {
  uint value;
} result;

layout (binding = 0) buffer TestData_1 {
  int data[3];
} s;

int runTest() {
  return atomicOr(s.data[(0u + uint(constants.zero))], 0);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  result.value = uint(runTest());
  return;
}
void main() {
  tint_symbol();
}


