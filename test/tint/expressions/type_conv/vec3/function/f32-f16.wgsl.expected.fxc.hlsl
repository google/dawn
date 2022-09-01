SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  const float3 tint_symbol = m();
  vector<float16_t, 3> v = vector<float16_t, 3>(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000024BEE975800(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

