SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 2, 3> m = matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001F06D082D20(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

