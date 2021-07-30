SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 1u;
  while (true) {
    var_1 = 2u;
    var_1 = 3u;
    switch(42u) {
      case 40u: {
        var_1 = 4u;
        {
          var_1 = 6u;
        }
        continue;
        break;
      }
      default: {
        break;
      }
    }
    var_1 = 5u;
    {
      var_1 = 6u;
    }
  }
  var_1 = 7u;
  return;
}

void main() {
  main_1();
  return;
}
warning: DXIL.dll not found.  Resulting DXIL will not be signed for use in release environments.

error: validation errors
tint_jbcKmW:30: error: Loop must have break.
Validation failed.



