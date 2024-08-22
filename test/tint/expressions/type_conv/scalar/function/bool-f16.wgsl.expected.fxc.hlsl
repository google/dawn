SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool m() {
  t = true;
  return bool(t);
}

void f() {
  bool tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
C:\src\dawn\Shader@0x000002C5748C6760(15,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000002C5748C6760(15,13): error X3000: unrecognized identifier 'v'

