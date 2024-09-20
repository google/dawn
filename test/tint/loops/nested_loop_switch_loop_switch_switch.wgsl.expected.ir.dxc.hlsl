
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
      bool tint_continue = false;
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
              bool tint_continue = false;
              switch(j) {
                case int(0):
                {
                  tint_continue = true;
                  break;
                }
                case int(1):
                {
                  bool tint_continue_1 = false;
                  switch(k) {
                    case int(0):
                    {
                      tint_continue_1 = true;
                      break;
                    }
                    default:
                    {
                      break;
                    }
                  }
                  if (tint_continue_1) {
                    tint_continue = true;
                    break;
                  }
                  break;
                }
                default:
                {
                  break;
                }
              }
              if (tint_continue) {
                {
                  j = (j + int(2));
                }
                continue;
              }
              {
                j = (j + int(2));
              }
              continue;
            }
          }
          tint_continue = true;
          break;
        }
        default:
        {
          break;
        }
      }
      if (tint_continue) {
        {
          i = (i + int(2));
        }
        continue;
      }
      {
        i = (i + int(2));
      }
      continue;
    }
  }
}

