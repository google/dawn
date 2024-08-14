SKIP: FAILED


static float16_t u = float16_t(1.0h);
void f() {
  float v = float(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x00000271581CFBA0(2,8-16): error X3000: unrecognized identifier 'float16_t'

