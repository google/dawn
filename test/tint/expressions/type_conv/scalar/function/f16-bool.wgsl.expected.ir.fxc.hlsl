SKIP: FAILED


static float16_t t = float16_t(0.0h);
float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

void f() {
  bool v = bool(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001F06ABF1F60(2,8-16): error X3000: unrecognized identifier 'float16_t'

