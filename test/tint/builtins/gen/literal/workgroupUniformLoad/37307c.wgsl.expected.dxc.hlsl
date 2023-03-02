uint tint_workgroupUniformLoad(inout uint p) {
  GroupMemoryBarrierWithGroupSync();
  const uint result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared uint arg_0;

void workgroupUniformLoad_37307c() {
  uint res = tint_workgroupUniformLoad(arg_0);
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
