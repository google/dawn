SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v = float16_t(0.0h);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001B65B3665B0(7,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000001B65B3665B0(7,13): error X3000: unrecognized identifier 'v'

