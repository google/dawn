SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  const float16_t r = (a / b);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000247BCDC5A20(3,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000247BCDC5A20(3,13): error X3000: unrecognized identifier 'a'

