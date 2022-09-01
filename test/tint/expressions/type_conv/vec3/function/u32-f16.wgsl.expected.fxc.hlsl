SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint t = 0u;

uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

void f() {
  const uint3 tint_symbol = m();
  vector<float16_t, 3> v = vector<float16_t, 3>(tint_symbol);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x0000022F414977C0(15,10-18): error X3000: syntax error: unexpected token 'float16_t'

