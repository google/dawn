groupshared int arg_0;
RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicSub_77883a() {
  int atomic_result = 0;
  InterlockedAdd(arg_0, -(1), atomic_result);
  int res = atomic_result;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    int atomic_result_1 = 0;
    InterlockedExchange(arg_0, 0, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicSub_77883a();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
