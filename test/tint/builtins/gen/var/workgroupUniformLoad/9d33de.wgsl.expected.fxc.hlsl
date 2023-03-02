int tint_workgroupUniformLoad(inout int p) {
  GroupMemoryBarrierWithGroupSync();
  const int result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared int arg_0;

void workgroupUniformLoad_9d33de() {
  int res = tint_workgroupUniformLoad(arg_0);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_0 = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  workgroupUniformLoad_9d33de();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
