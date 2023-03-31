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

ByteAddressBuffer rarr : register(t0);

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
DXC validation failure:
shader.hlsl:3:28: error: vector element index '3' is out of bounds
  const int x = int2(1, 2)[idx];
                           ^
shader.hlsl:8:69: error: matrix row index '4' is out of bounds
  const float2 x = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))[idx];
                                                                    ^
shader.hlsl:14:17: error: array index 3 is out of bounds
  const int x = arr[idx];
                ^
shader.hlsl:12:3: note: array 'arr' declared here
  const int arr[2] = {1, 2};
  ^


