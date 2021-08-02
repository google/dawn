[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  switch(i) {
    case 0: {
      /* fallthrough */
      {
        break;
      }
    }
    default: {
      break;
    }
  }
  return;
}
