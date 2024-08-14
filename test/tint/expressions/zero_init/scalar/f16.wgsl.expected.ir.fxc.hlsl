SKIP: FAILED


void f() {
  float16_t v = float16_t(0.0h);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022650BBFFD0(3,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x0000022650BBFFD0(3,13): error X3000: unrecognized identifier 'v'

