SKIP: FAILED

static uint x_1 = 0u;
static bool x_17 = false;

void main_1() {
  const bool x_101 = x_17;
  while (true) {
    uint x_2_phi = 0u;
    uint x_5_phi = 0u;
    x_2_phi = 0u;
    x_5_phi = 1u;
    while (true) {
      uint x_7 = 0u;
      const uint x_5 = x_5_phi;
      const uint x_4 = (x_2_phi + 1u);
      const uint x_6 = (x_4 + 1u);
      if (x_101) {
        break;
      }
      {
        x_7 = (x_4 + x_6);
        x_2_phi = x_4;
        x_5_phi = x_7;
      }
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
tint_AU073z:29: error: Loop must have break.
Validation failed.



