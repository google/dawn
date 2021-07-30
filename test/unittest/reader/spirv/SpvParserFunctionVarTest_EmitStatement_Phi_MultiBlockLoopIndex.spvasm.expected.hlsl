SKIP: FAILED

static uint x_1 = 0u;
static bool x_7 = false;
static bool x_8 = false;

void main_1() {
  while (true) {
    uint x_2_phi = 0u;
    uint x_3_phi = 0u;
    const bool x_101 = x_7;
    const bool x_102 = x_8;
    x_2_phi = 0u;
    x_3_phi = 1u;
    if (x_101) {
      break;
    }
    while (true) {
      uint x_4 = 0u;
      const uint x_2 = x_2_phi;
      const uint x_3 = x_3_phi;
      if (x_102) {
        break;
      }
      {
        x_4 = (x_2 + 1u);
        x_2_phi = x_4;
        x_3_phi = x_3;
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
tint_eXjRIC:33: error: Loop must have break.
Validation failed.



