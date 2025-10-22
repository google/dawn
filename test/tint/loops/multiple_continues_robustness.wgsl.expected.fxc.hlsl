
[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while((i < int(2))) {
      bool tint_continue = false;
      switch(i) {
        case int(0):
        {
          tint_continue = true;
          break;
        }
        case int(1):
        {
          tint_continue = true;
          break;
        }
        case int(2):
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
          i = asint((asuint(i) + asuint(int(1))));
        }
        continue;
      }
      {
        i = asint((asuint(i) + asuint(int(1))));
      }
      continue;
    }
  }
}

