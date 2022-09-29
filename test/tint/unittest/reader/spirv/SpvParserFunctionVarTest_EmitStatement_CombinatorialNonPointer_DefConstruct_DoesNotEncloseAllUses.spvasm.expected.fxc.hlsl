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
      if (false) {
        break;
      }
    }
  }
  x_1 = 5u;
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x00000292BFA3C190(5,17-20): error X3696: infinite loop detected - loop never exits

