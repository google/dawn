SKIP: FAILED


static int u = 1;
void f() {
  float16_t v = float16_t(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000024E43DE4AB0(4,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x0000024E43DE4AB0(4,13): error X3000: unrecognized identifier 'v'

