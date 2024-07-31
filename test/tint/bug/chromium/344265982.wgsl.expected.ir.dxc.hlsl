
RWByteAddressBuffer buffer : register(u0);
void foo() {
  {
    int i = 0;
    while(true) {
      if ((i < 4)) {
      } else {
        break;
      }
      uint v = (uint(i) * 4u);
      switch(asint(buffer.Load((0u + v)))) {
        case 1:
        {
          {
            i = (i + 1);
          }
          continue;
        }
        default:
        {
          uint v_1 = ((0u + v) + (uint(i) * 4u));
          buffer.Store(v_1, asuint(2));
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

