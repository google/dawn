SKIP: FAILED


static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  float16_t v = float16_t(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000018946914450(9,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x0000018946914450(9,13): error X3000: unrecognized identifier 'v'

