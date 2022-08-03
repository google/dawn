SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float16_t get_f16() {
  return float16_t(1.0h);
}

void f() {
  const float16_t tint_symbol = get_f16();
  vector<float16_t, 2> v2 = vector<float16_t, 2>((tint_symbol).xx);
  const float16_t tint_symbol_1 = get_f16();
  vector<float16_t, 3> v3 = vector<float16_t, 3>((tint_symbol_1).xxx);
  const float16_t tint_symbol_2 = get_f16();
  vector<float16_t, 4> v4 = vector<float16_t, 4>((tint_symbol_2).xxxx);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001EAED2412C0(6,1-9): error X3000: unrecognized identifier 'float16_t'

