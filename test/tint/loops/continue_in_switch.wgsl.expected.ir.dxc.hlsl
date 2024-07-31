
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

