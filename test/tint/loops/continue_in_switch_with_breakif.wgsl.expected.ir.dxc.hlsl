
[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  {
    while(true) {
      switch(i) {
        case 0:
        {
          {
            i = (i + 1);
            if ((i >= 4)) { break; }
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
        if ((i >= 4)) { break; }
      }
      continue;
    }
  }
}

