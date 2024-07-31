SKIP: FAILED


static uint var_1 = 0u;
void main_1() {
  var_1 = 0u;
  {
    while(true) {
      var_1 = 1u;
      if (false) {
        break;
      }
      {
      }
      continue;
    }
  }
  var_1 = 5u;
}

void main() {
  main_1();
}

FXC validation failure:
c:\src\dawn\Shader@0x000001D411DF0730(6,11-14): error X3696: infinite loop detected - loop never exits

