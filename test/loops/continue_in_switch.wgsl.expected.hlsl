SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      switch(i) {
        case 0: {
          continue;
          break;
        }
        default: {
          break;
        }
      }
    }
  }
  return;
}
C:\src\tint\test\Shader@0x000001C513B5E750(7,11-19): error X3708: continue cannot be used in a switch

