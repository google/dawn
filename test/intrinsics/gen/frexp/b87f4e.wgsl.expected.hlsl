groupshared int4 arg_1;

void frexp_b87f4e() {
  float4 tint_tmp;
  float4 tint_tmp_1 = frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = int4(tint_tmp);
  float4 res = tint_tmp_1;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  const uint local_invocation_index = tint_symbol.local_invocation_index;
  if ((local_invocation_index == 0u)) {
    arg_1 = int4(0, 0, 0, 0);
  }
  GroupMemoryBarrierWithGroupSync();
  frexp_b87f4e();
  return;
}
