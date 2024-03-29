groupshared int S;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    S = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

int func_S() {
  return S;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int r = func_S();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
