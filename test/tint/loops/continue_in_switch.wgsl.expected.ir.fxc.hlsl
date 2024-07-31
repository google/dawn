SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      switch(i) {
        case 0:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}

FXC validation failure:
c:\src\dawn\Shader@0x0000028661043900(17,11-19): error X3708: continue cannot be used in a switch

