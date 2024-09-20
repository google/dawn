
[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol = 0u;
  {
    while(true) {
      bool tint_continue = false;
      switch(int(2)) {
        case int(1):
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
          if (true) { break; }
        }
        continue;
      }
      tint_symbol = (tint_symbol + 1u);
      {
        if (true) { break; }
      }
      continue;
    }
  }
}

