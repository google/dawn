
[numthreads(1, 1, 1)]
void main() {
  int j = int(0);
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
          switch(j) {
            case int(0):
            {
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
          break;
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

