
ByteAddressBuffer S : register(t0);
float4 func(uint pointer_indices[1]) {
  return asfloat(S.Load4((0u + (uint(pointer_indices[0u]) * 16u))));
}

[numthreads(1, 1, 1)]
void main() {
  uint v[1] = (uint[1])0;
  float4 r = func(v);
}

