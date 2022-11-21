groupshared int W[246];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 246u); idx = (idx + 1u)) {
      const uint i = idx;
      W[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  W[0] = 42;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
