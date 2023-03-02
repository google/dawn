struct str {
  int i;
};

groupshared str S[4];

void func_S_X(uint pointer[1]) {
  const str tint_symbol_4 = (str)0;
  S[pointer[0]] = tint_symbol_4;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      const uint i_1 = idx;
      const str tint_symbol_2 = (str)0;
      S[i_1] = tint_symbol_2;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  const uint tint_symbol_3[1] = {2u};
  func_S_X(tint_symbol_3);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
