SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int u = 1;

void f() {
  float16_t v = float16_t(u);
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D609400980(9,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x000001D609400980(9,13): error X3000: unrecognized identifier 'v'

