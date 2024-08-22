SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 2, 3> u = matrix<float16_t, 2, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h)));
FXC validation failure:
C:\src\dawn\Shader@0x000001A568525240(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

