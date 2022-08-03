SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  const float16_t r = (float16_t(1.0h) % float16_t(2.0h));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000023722D34110(3,9-17): error X3000: unrecognized identifier 'float16_t'

