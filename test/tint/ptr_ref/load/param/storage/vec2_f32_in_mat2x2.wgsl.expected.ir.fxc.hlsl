
ByteAddressBuffer S : register(t0);
float2 func(uint pointer_indices[1]) {
  return asfloat(S.Load2((0u + (uint(pointer_indices[0u]) * 8u))));
}

[numthreads(1, 1, 1)]
void main() {
  uint v[1] = {uint(int(1))};
  float2 r = func(v);
}

