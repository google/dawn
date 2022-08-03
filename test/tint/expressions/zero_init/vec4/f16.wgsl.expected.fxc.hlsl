SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 4> v = (float16_t(0.0h)).xxxx;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000025C4B7127F0(7,10-18): error X3000: syntax error: unexpected token 'float16_t'

