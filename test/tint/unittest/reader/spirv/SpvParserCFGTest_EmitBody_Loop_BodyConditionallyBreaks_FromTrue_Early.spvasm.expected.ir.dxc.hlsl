SKIP: INVALID


static uint var_1 = 0u;
void main_1() {
  {
    while(true) {
      var_1 = 1u;
      if (false) {
        break;
      }
      var_1 = 3u;
      {
        var_1 = 2u;
      }
      continue;
    }
  }
}

void main() {
  main_1();
}

DXC validation failure:
error: validation errors
hlsl.hlsl:19: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
