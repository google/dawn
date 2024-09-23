
[numthreads(1, 1, 1)]
void f() {
  int i = int(0);
  {
    while(true) {
      switch(i) {
        case int(0):
        {
          {
            i = (i + int(1));
            if ((i >= int(4))) { break; }
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
        if ((i >= int(4))) { break; }
      }
      continue;
    }
  }
}

