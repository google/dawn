groupshared int zero[3];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 3u); idx = (idx + 1u)) {
      const uint i = idx;
      zero[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int v[3] = zero;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
