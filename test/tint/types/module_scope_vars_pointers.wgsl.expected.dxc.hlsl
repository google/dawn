static float p = 0.0f;
groupshared float w;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    w = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  const float x = (p + w);
  p = x;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
