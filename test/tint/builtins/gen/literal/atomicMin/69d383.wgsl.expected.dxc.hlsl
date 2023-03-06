groupshared uint arg_0;
RWByteAddressBuffer prevent_dce : register(u0, space2);

void atomicMin_69d383() {
  uint atomic_result = 0u;
  InterlockedMin(arg_0, 1u, atomic_result);
  uint res = atomic_result;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    uint atomic_result_1 = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicMin_69d383();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
