SKIP: FAILED


void f() {
  float16_t v[4] = (float16_t[4])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000023BFC8B0220(3,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x0000023BFC8B0220(3,13): error X3000: unrecognized identifier 'v'

