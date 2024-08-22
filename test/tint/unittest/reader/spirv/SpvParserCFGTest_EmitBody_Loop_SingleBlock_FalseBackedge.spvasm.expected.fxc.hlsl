SKIP: FAILED

static uint var_1 = 0u;

void main_1() {
  var_1 = 0u;
  while (true) {
    var_1 = 1u;
    if (false) {
      break;
    }
  }
  var_1 = 999u;
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D3E6732650(5,10-13): error X3696: infinite loop detected - loop never exits

