
RWByteAddressBuffer buffer : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int i = asint(buffer.Load(0u));
  {
    while(true) {
      {
        {
          while(true) {
            if ((i > int(5))) {
              i = (i * int(2));
              break;
            } else {
              i = (i * int(2));
              break;
            }
            /* unreachable */
          }
        }
        if ((i > int(10))) { break; }
      }
      continue;
    }
  }
  buffer.Store(0u, asuint(i));
}

