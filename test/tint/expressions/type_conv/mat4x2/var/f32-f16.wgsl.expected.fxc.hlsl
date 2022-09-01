SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float4x2 u = float4x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f), float2(7.0f, 8.0f));

void f() {
  matrix<float16_t, 4, 2> v = matrix<float16_t, 4, 2>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x000002880E1B53D0(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

