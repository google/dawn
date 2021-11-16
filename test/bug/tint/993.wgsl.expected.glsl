SKIP: FAILED

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
  int atomic_result = 0;
  InterlockedOr(s.data[(0u + uint(constants.zero))], 0, atomic_result);
  return atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  result.value = uint(runTest());
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:19: 'InterlockedOr' : no matching overloaded function found 
ERROR: 0:19: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



