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
  var_1 = 999u;
}

void main() {
  main_1();
}

FXC validation failure:
C:\src\dawn\Shader@0x00000296341533A0(6,11-14): error X3696: infinite loop detected - loop never exits

