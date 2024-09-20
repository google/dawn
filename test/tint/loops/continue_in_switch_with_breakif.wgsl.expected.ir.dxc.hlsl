
[numthreads(1, 1, 1)]
void f() {
  int i = int(0);
  {
    while(true) {
      bool tint_continue = false;
      switch(i) {
        case int(0):
        {
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
          i = (i + int(1));
          if ((i >= int(4))) { break; }
        }
        continue;
      }
      {
        i = (i + int(1));
        if ((i >= int(4))) { break; }
      }
      continue;
    }
  }
}

