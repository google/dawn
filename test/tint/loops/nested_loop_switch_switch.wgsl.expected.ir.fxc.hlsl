SKIP: FAILED


[numthreads(1, 1, 1)]
void main() {
  int j = 0;
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
          switch(j) {
            case 0:
            {
              {
                i = (i + 2);
              }
              continue;
            }
            default:
            {
              break;
            }
          }
          break;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 2);
      }
      continue;
    }
  }
}

FXC validation failure:
c:\src\dawn\Shader@0x000002665D442960(21,15-23): error X3708: continue cannot be used in a switch

