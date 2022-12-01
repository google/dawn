struct str {
  float4 i;
};

groupshared str S;

float4 func_S_i() {
  return S.i;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    const str tint_symbol_2 = (str)0;
    S = tint_symbol_2;
  }
  GroupMemoryBarrierWithGroupSync();
  const float4 r = func_S_i();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
