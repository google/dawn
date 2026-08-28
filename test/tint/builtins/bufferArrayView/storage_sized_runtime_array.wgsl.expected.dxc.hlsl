
RWByteAddressBuffer v : register(u0);
[numthreads(1, 1, 1)]
void main() {
  min(uint(int(0)), (8u - 1u));
  v.Store(0u, 2u);
}

