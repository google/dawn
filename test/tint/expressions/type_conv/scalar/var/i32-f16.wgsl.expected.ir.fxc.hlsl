SKIP: FAILED


static int u = 1;
void f() {
  float16_t v = float16_t(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x00000208DA33AFB0(4,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x00000208DA33AFB0(4,13): error X3000: unrecognized identifier 'v'

