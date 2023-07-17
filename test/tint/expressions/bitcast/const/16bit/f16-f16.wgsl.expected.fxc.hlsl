SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const float16_t b = float16_t(1.0h);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\bitcast\Shader@0x00000174D90470D0(3,9-17): error X3000: unrecognized identifier 'float16_t'

