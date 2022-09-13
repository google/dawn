SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  {
    [loop] for(; true; ) {
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000023A01048490(5,12-24): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000023A01048490(5,12-24): warning X3551: infinite loop detected - loop writes no values
C:\src\dawn\test\tint\Shader@0x0000023A01048490(5,18-21): error X3696: infinite loop detected - loop never exits

