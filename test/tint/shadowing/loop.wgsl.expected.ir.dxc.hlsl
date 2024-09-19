
RWByteAddressBuffer output : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  int i = int(0);
  {
    while(true) {
      int x = asint(output.Load((0u + (uint(i) * 4u))));
      {
        int x = asint(output.Load((0u + (uint(x) * 4u))));
        i = (i + x);
        if ((i > int(10))) { break; }
      }
      continue;
    }
  }
  output.Store(0u, asuint(i));
}

