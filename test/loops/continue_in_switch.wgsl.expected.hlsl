[numthreads(1, 1, 1)]
void f() {
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      switch(i) {
        case 0: {
          continue;
          break;
        }
        default: {
          break;
        }
      }
    }
  }
  return;
}
