SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  [loop] while (true) {
    var_1 = 1u;
    if (false) {
      break;
    }
    var_1 = 3u;
    {
      var_1 = 2u;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000027D4D5AB130(4,17-20): error X3696: infinite loop detected - loop never exits

