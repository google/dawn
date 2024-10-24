
RWByteAddressBuffer S : register(u0);
void func(uint pointer_indices[1]) {
  uint v = (0u + (uint(pointer_indices[0u]) * 8u));
  S.Store2(v, asuint((0.0f).xx));
}

[numthreads(1, 1, 1)]
void main() {
  uint v_1[1] = {uint(int(1))};
  func(v_1);
}

