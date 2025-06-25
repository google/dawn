struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared bool arg_0;
int workgroupUniformLoad_b75d53() {
  GroupMemoryBarrierWithGroupSync();
  bool v = arg_0;
  GroupMemoryBarrierWithGroupSync();
  bool res = v;
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = false;
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_b75d53()));
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

