
RWByteAddressBuffer a : register(u0);
[numthreads(1, 1, 1)]
void main() {
  a.Store(4u, (a.Load(4u) - 1u));
  a.Store(8u, (a.Load(8u) - 1u));
}

