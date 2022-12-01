struct str {
  int arr[4];
};

groupshared str S;

void func_S_arr() {
  const int tint_symbol_2[4] = (int[4])0;
  S.arr = tint_symbol_2;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      const uint i = idx;
      S.arr[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  func_S_arr();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
