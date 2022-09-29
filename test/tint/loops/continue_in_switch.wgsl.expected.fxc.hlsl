[numthreads(1, 1, 1)]
void f() {
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      bool tint_continue = false;
      switch(i) {
        case 0: {
          {
            tint_continue = true;
            break;
          }
          break;
        }
        default: {
          break;
        }
      }
      if (tint_continue) {
        continue;
      }
    }
  }
  return;
}
