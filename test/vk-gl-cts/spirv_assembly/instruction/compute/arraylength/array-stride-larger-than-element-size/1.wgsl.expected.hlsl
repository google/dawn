RWByteAddressBuffer x_4 : register(u1, space0);
RWByteAddressBuffer x_7 : register(u0, space0);

void main_1() {
  uint i = 0u;
  x_4.Store(0u, asuint(1));
  i = 0u;
  {
    for(; (i < 512u); i = (i + asuint(1))) {
      const uint x_39 = x_7.Load((4u * (i * 2u)));
      if ((x_39 != i)) {
        x_4.Store(0u, asuint(0));
      }
    }
  }
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
