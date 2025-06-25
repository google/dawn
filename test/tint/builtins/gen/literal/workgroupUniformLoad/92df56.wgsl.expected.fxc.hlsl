struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared int arg_0;
int workgroupUniformLoad_92df56() {
  GroupMemoryBarrierWithGroupSync();
  int v = int(0);
  InterlockedOr(arg_0, int(0), v);
  int v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  int res = v_1;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_2 = int(0);
    InterlockedExchange(arg_0, int(0), v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_92df56()));
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

