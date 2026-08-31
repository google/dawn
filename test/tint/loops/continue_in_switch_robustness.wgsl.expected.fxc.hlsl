
[numthreads(1, 1, 1)]
void f() {
  {
    int i = int(0);
    for( ; (i < int(4)); i = asint((asuint(i) + 1u))) {
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
        continue;
      }
    }
  }
}

