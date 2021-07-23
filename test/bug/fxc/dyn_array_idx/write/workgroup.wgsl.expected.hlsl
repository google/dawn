cbuffer cbuffer_ubo : register(b0, space0) {
  uint4 ubo[1];
};

struct S {
  int data[64];
};

RWByteAddressBuffer result : register(u1, space0);
groupshared S s;

struct tint_symbol_2 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void f(tint_symbol_2 tint_symbol_1) {
  const uint local_invocation_index = tint_symbol_1.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    {
      for(int i = 0; (i < 64); i = (i + 1)) {
        s.data[i] = 0;
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  s.data[asint(ubo[0].x)] = 1;
  result.Store(0u, asuint(s.data[3]));
  return;
}
