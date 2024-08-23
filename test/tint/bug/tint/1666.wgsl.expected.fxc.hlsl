SKIP: FAILED

void tint_symbol() {
  int idx = 3;
  int x = int2(1, 2)[idx];
}

void tint_symbol_1() {
  int idx = 4;
  float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[idx];
}

void fixed_size_array() {
  int arr[2] = {1, 2};
  int idx = 3;
  int x = arr[idx];
}

ByteAddressBuffer rarr : register(t0);

void runtime_size_array() {
  int idx = -1;
  float x = asfloat(rarr.Load((4u * uint(idx))));
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
<scrubbed_path>(3,11-25): error X3504: array index out of bounds


tint executable returned error: exit status 1
