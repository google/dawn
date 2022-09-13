SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  [loop] while (true) {
    if (false) {
      break;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x000001FE0F8E0000(4,10-21): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x000001FE0F8E0000(4,10-21): warning X3551: infinite loop detected - loop writes no values
C:\src\dawn\test\tint\Shader@0x000001FE0F8E0000(4,17-20): error X3696: infinite loop detected - loop never exits

