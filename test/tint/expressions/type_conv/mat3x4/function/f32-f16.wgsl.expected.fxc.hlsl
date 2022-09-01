SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float3x4 m() {
  t = (t + 1.0f);
  return float3x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f));
}

void f() {
  const float3x4 tint_symbol = m();
  matrix<float16_t, 3, 4> v = matrix<float16_t, 3, 4>(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000001C84A702630(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

