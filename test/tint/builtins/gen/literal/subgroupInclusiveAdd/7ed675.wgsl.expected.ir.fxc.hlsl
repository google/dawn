SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveAdd_7ed675() {
  uint res = (WavePrefixSum(1u) + 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupInclusiveAdd_7ed675());
}

