SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 4, 4> m = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x00000277C3AD2D20(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

