SKIP: FAILED


[numthreads(1, 1, 1)]
void main() {
  {
    int i = 0;
    while(true) {
      if ((i < 2)) {
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
        case 1:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        case 2:
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
c:\src\dawn\Shader@0x000002D89A881430(17,11-19): error X3708: continue cannot be used in a switch

