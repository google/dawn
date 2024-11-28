
ByteAddressBuffer rarr : register(t0);
void tint_symbol() {
  int idx = int(3);
  int x = int2(int(1), int(2))[min(uint(idx), 1u)];
}

void tint_symbol_1() {
  int idx = int(4);
  float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[min(uint(idx), 1u)];
}

void fixed_size_array() {
  int arr[2] = {int(1), int(2)};
  int idx = int(3);
  int x = arr[min(uint(idx), 1u)];
}

void runtime_size_array() {
  int idx = int(-1);
  uint v = 0u;
  rarr.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  float x = asfloat(rarr.Load((0u + (min(uint(idx), v_1) * 4u))));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol();
  tint_symbol_1();
  fixed_size_array();
  runtime_size_array();
}

