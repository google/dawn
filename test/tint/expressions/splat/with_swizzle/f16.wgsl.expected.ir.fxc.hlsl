SKIP: FAILED


void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(1.0h);
  float16_t c = float16_t(1.0h);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x00000229610217A0(3,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x00000229610217A0(3,13): error X3000: unrecognized identifier 'a'

