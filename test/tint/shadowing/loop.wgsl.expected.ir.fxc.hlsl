SKIP: FAILED


RWByteAddressBuffer output : register(u0);
[numthreads(1, 1, 1)]
void foo() {
  int i = 0;
  {
    while(true) {
      int x = asint(output.Load((0u + (uint(i) * 4u))));
      {
        int x = asint(output.Load((0u + (uint(x) * 4u))));
        i = (i + x);
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  output.Store(0u, asuint(i));
}

FXC validation failure:
<scrubbed_path>(10,42-53): warning X4000: use of potentially uninitialized variable (x)
<scrubbed_path>(10,42-53): error X4575: reading uninitialized value


tint executable returned error: exit status 1
