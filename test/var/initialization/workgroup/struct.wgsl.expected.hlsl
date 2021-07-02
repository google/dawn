struct S {
  int a;
  float b;
};

groupshared S v;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    const S tint_symbol_2 = (S)0;
    v = tint_symbol_2;
  }
  GroupMemoryBarrierWithGroupSync();
  v;
  return;
}
