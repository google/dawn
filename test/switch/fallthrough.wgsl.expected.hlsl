SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  switch(i) {
    case 0: {
      /* fallthrough */
    }
    default: {
      break;
    }
  }
  return;
}
C:\src\tint\test\Shader@0x000001AF1A4F6940(5,5): error X3533: non-empty case statements must have break or return

