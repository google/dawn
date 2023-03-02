SKIP: FAILED

static uint x_1 = 0u;

void main_1() {
  x_1 = 0u;
  while (true) {
    uint x_2 = 0u;
    x_1 = 1u;
    if (false) {
      break;
    }
    x_1 = 3u;
    if (true) {
      x_2 = 2u;
    } else {
      return;
    }
    x_1 = x_2;
    {
      x_1 = 4u;
      if (false) { break; }
    }
  }
  x_1 = 5u;
  return;
}

void main() {
  main_1();
  return;
}
DXC validation failure:
warning: DXIL signing library (dxil.dll,libdxil.so) not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
shader.hlsl:27: error: Loop must have break.
Validation failed.



