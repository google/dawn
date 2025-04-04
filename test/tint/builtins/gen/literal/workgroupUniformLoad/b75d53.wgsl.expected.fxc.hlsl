groupshared bool arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = false;
  }
  GroupMemoryBarrierWithGroupSync();
}

bool tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  bool result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0);

int workgroupUniformLoad_b75d53() {
  bool res = tint_workgroupUniformLoad_arg_0();
  return (all((res == false)) ? 1 : 0);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_b75d53()));
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
