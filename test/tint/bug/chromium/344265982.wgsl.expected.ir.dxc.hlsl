
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      switch(asint(buffer.Load((0u + (uint(i) * 4u))))) {
        case 1:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        default:
        {
          uint v = (0u + (uint(i) * 4u));
          buffer.Store(v, asuint(2));
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
}

void main() {
  foo();
}

