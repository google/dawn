
[numthreads(1, 1, 1)]
void main() {
  int k = int(0);
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      switch(i) {
        case int(0):
        {
          {
            int j = int(0);
            while(true) {
              if ((j < int(2))) {
              } else {
                break;
              }
              switch(j) {
                case int(0):
                {
                  {
                    j = (j + int(2));
                  }
                  continue;
                }
                case int(1):
                {
                  switch(k) {
                    case int(0):
                    {
                      {
                        j = (j + int(2));
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
                j = (j + int(2));
              }
              continue;
            }
          }
          {
            i = (i + int(2));
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + int(2));
      }
      continue;
    }
  }
}

