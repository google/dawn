SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = a;
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001C74E726820(3,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000001C74E726820(3,13): error X3000: unrecognized identifier 'a'

