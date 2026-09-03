
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  min(0u, (8u - 1u));
  v.Store(0u, 2u);
}

