float tint_workgroupUniformLoad(inout float p) {
  GroupMemoryBarrierWithGroupSync();
  const float result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared float arg_0;
RWByteAddressBuffer prevent_dce : register(u0, space2);

void workgroupUniformLoad_7a857c() {
  float res = tint_workgroupUniformLoad(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  {
    arg_0 = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  workgroupUniformLoad_7a857c();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
