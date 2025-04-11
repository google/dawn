groupshared int arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    int atomic_result = 0;
    InterlockedExchange(arg_0, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

int tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  int atomic_result_1 = 0;
  InterlockedOr(arg_0, 0, atomic_result_1);
  int result = atomic_result_1;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0);

int workgroupUniformLoad_92df56() {
  int res = tint_workgroupUniformLoad_arg_0();
  return res;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_92df56()));
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
