SKIP: FAILED

void tint_symbol() {
  const int idx = 3;
  const int x = int2(1, 2)[idx];
}

void tint_symbol_1() {
  const int idx = 4;
  const float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[idx];
}

void fixed_size_array() {
  const int arr[2] = {1, 2};
  const int idx = 3;
  const int x = arr[idx];
}

ByteAddressBuffer rarr : register(t0, space0);

void runtime_size_array() {
  const int idx = -1;
  const float x = asfloat(rarr.Load((4u * uint(idx))));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol();
  tint_symbol_1();
  fixed_size_array();
  runtime_size_array();
  return;
}
FXC validation failure:
T:\tmp\dawn-temp\dawn-src\test\tint\Shader@0x0000019913EB4550(3,17-31): error X3030: array index out of bounds
