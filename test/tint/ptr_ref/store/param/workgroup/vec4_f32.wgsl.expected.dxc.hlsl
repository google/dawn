groupshared float4 S;

void func_S() {
  S = (0.0f).xxxx;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    S = (0.0f).xxxx;
  }
  GroupMemoryBarrierWithGroupSync();
  func_S();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
