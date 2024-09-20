
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(4))) {
      } else {
        break;
      }
      bool tint_continue = false;
      switch(asint(buffer.Load((0u + (uint(i) * 4u))))) {
        case int(1):
        {
          tint_continue = true;
          break;
        }
        default:
        {
          uint v = (0u + (uint(i) * 4u));
          buffer.Store(v, asuint(int(2)));
          break;
        }
      }
      if (tint_continue) {
        {
          i = (i + int(1));
        }
        continue;
      }
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

void main() {
  foo();
}

