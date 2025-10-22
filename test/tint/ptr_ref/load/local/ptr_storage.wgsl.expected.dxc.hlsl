
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  int u = asint((asuint(asint(v.Load(0u))) + asuint(int(1))));
}

