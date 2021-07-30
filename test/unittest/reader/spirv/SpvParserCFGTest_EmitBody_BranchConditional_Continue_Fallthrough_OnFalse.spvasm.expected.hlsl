SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  while (true) {
    var_1 = 1u;
    var_1 = 2u;
    switch(42u) {
      case 40u: {
        var_1 = 40u;
        if (false) {
          {
            var_1 = 4u;
          }
          continue;
        }
        /* fallthrough */
      }
      case 50u: {
        var_1 = 50u;
        break;
      }
      default: {
        break;
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
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
tint_cTIG3E:36: error: Loop must have break.
Validation failed.



