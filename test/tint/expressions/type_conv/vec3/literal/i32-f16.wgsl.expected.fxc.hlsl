SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
FXC validation failure:
C:\src\dawn\Shader@0x000002801B5E80C0(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

