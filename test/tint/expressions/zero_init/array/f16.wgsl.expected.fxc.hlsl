SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v[4] = (float16_t[4])0;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000273A5B065E0(7,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x00000273A5B065E0(7,13): error X3000: unrecognized identifier 'v'

