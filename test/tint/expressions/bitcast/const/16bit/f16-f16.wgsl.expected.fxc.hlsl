SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  float16_t b = float16_t(1.0h);
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001FD010977A0(3,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000001FD010977A0(3,13): error X3000: unrecognized identifier 'b'

