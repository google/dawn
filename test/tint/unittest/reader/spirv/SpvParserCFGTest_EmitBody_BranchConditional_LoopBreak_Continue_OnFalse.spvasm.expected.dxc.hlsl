SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  while (true) {
    var_1 = 1u;
    if (true) {
      var_1 = 2u;
      if (false) {
        break;
      } else {
        {
          var_1 = 4u;
        }
        continue;
      }
    }
    var_1 = 3u;
    {
      var_1 = 4u;
    }
  }
  var_1 = 5u;
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



