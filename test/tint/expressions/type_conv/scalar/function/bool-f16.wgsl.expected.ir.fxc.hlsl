SKIP: FAILED


static bool t = false;
bool m() {
  t = true;
  return bool(t);
}

void f() {
  float16_t v = float16_t(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000021699710A70(9,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x0000021699710A70(9,13): error X3000: unrecognized identifier 'v'

