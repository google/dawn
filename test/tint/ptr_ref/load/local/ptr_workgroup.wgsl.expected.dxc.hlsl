groupshared int i;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    i = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  i = 123;
  const int u = (i + 1);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
