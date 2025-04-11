struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared uint arg_0;
uint workgroupUniformLoad_7ea8de() {
  GroupMemoryBarrierWithGroupSync();
  uint v = 0u;
  InterlockedOr(arg_0, 0u, v);
  uint v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  uint res = v_1;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    uint v_2 = 0u;
    InterlockedExchange(arg_0, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, workgroupUniformLoad_7ea8de());
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

