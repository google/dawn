void set_vector_float2x4(inout float2x4 mat, int col, float4 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
  }
}

groupshared float2x4 S;

void func_S_X(uint pointer[1]) {
  set_vector_float2x4(S, pointer[0], (0.0f).xxxx);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    S = float2x4((0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
  const uint tint_symbol_2[1] = {1u};
  func_S_X(tint_symbol_2);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
