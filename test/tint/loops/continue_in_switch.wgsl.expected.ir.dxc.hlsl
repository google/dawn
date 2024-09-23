
[numthreads(1, 1, 1)]
void f() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(4))) {
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

