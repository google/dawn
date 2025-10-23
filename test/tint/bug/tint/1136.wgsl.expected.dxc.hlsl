
RWByteAddressBuffer buffer : register(u0);
[numthreads(1, 1, 1)]
void main() {
  buffer.Store(0u, (buffer.Load(0u) + 1u));
}

