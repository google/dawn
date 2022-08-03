SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 3, 4> m = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000020DF1435780(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

