SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float2 m() {
  t = 1.0f;
  return float2((t).xx);
}

void f() {
  const float2 tint_symbol = m();
  vector<float16_t, 2> v = vector<float16_t, 2>(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000018B591277C0(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

