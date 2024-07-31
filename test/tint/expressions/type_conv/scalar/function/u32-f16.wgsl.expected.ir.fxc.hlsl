SKIP: FAILED


static uint t = 0u;
uint m() {
  t = 1u;
  return uint(t);
}

void f() {
  float16_t v = float16_t(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000002995A904370(9,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x000002995A904370(9,13): error X3000: unrecognized identifier 'v'

