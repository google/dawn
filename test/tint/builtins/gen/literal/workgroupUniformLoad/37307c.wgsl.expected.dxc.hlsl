groupshared uint arg_0;

uint tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  const uint result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void workgroupUniformLoad_37307c() {
  uint res = tint_workgroupUniformLoad_arg_0();
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_0 = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  workgroupUniformLoad_37307c();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
