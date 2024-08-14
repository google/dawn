SKIP: FAILED


static float16_t u = float16_t(1.0h);
int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (-2147483648))) : (2147483647));
}

void f() {
  int v = tint_f16_to_i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x000001733E99DE70(2,8-16): error X3000: unrecognized identifier 'float16_t'

