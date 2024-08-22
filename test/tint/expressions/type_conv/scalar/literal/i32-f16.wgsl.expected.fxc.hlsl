SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t u = float16_t(1.0h);
FXC validation failure:
C:\src\dawn\Shader@0x00000214E76D6130(6,8-16): error X3000: unrecognized identifier 'float16_t'

