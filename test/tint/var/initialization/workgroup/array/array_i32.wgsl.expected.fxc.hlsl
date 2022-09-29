groupshared int zero[2][3];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 6u); idx = (idx + 1u)) {
      const uint i = (idx / 3u);
      const uint i_1 = (idx % 3u);
      zero[i][i_1] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int v[2][3] = zero;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
