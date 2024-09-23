
[numthreads(1, 1, 1)]
void main() {
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
            i = (i + int(1));
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

