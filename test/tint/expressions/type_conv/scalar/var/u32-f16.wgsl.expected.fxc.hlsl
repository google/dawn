SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint u = 1u;

void f() {
  float16_t v = float16_t(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x00000218582B09D0(9,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x00000218582B09D0(9,13): error X3000: unrecognized identifier 'v'

