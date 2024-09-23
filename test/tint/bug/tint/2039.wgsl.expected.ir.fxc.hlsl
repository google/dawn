
[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol = 0u;
  {
    while(true) {
      switch(int(2)) {
        case int(1):
        {
          {
            if (true) { break; }
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      tint_symbol = (tint_symbol + 1u);
      {
        if (true) { break; }
      }
      continue;
    }
  }
}

