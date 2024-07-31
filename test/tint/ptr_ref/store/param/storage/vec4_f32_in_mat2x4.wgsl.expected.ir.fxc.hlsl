
RWByteAddressBuffer S : register(u0);
void func(uint pointer_indices[1]) {
  uint v = (0u + (uint(pointer_indices[0u]) * 16u));
  S.Store4(v, asuint((0.0f).xxxx));
}

[numthreads(1, 1, 1)]
void main() {
  uint v_1[1] = (uint[1])0;
  func(v_1);
}

