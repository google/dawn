SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static matrix<float16_t, 4, 2> m = matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x0000021E1FA32CE0(6,15-23): error X3000: syntax error: unexpected token 'float16_t'

