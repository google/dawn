SKIP: FAILED

void main_1() {
  uint x_25 = 0u;
  const uint x_2 = (1u + 1u);
  x_25 = 1u;
  while (true) {
    {
      x_25 = x_2;
    }
  }
  x_25 = 2u;
  return;
}

void main() {
  main_1();
  return;
}
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
tint_yj82Xu:14: error: Loop must have break.
Validation failed.



