groupshared float2x3 v;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    v = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  }
  GroupMemoryBarrierWithGroupSync();
  v;
  return;
}
