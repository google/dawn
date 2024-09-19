
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = int(0);
    while(true) {
      if ((i < int(4))) {
      } else {
        break;
      }
      switch(asint(buffer.Load((0u + (uint(i) * 4u))))) {
        case int(1):
        {
          {
            i = (i + int(1));
          }
          continue;
        }
        default:
        {
          uint v = (0u + (uint(i) * 4u));
          buffer.Store(v, asuint(int(2)));
          break;
        }
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

