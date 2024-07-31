SKIP: FAILED


[numthreads(1, 1, 1)]
void main() {
  int k = 0;
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
            int j = 0;
            while(true) {
              if ((j < 2)) {
              } else {
                break;
              }
              switch(j) {
                case 0:
                {
                  {
                    j = (j + 2);
                  }
                  continue;
                }
                case 1:
                {
                  switch(k) {
                    case 0:
                    {
                      {
                        j = (j + 2);
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
                j = (j + 2);
              }
              continue;
            }
          }
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
      {
        i = (i + 2);
      }
      continue;
    }
  }
}

FXC validation failure:
c:\src\dawn\Shader@0x000001CB9A8AC650(28,19-27): error X3708: continue cannot be used in a switch

