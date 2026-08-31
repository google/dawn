
RWByteAddressBuffer S : register(u0);
void func(uint pointer_indices[1]) {
  S.Store2((0u + (pointer_indices[0u] * 8u)), (0u).xx);
}

[numthreads(1, 1, 1)]
void main() {
  uint v[1] = {1u};
  func(v);
}

