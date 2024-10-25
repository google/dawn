
ByteAddressBuffer rarr : register(t0);
void tint_symbol() {
  int idx = int(3);
  int x = int2(int(1), int(2))[idx];
}

void tint_symbol_1() {
  int idx = int(4);
  float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[idx];
}

void fixed_size_array() {
  int arr[2] = {int(1), int(2)};
  int idx = int(3);
  int x = arr[idx];
}

void runtime_size_array() {
  int idx = int(-1);
  float x = asfloat(rarr.Load((0u + (uint(idx) * 4u))));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol();
  tint_symbol_1();
  fixed_size_array();
  runtime_size_array();
}

