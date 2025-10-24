
RWByteAddressBuffer i : register(u0);
[numthreads(1, 1, 1)]
void main() {
  i.Store(0u, (i.Load(0u) + 1u));
}

