//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveAdd_8bbe75() {
  uint4 res = (WavePrefixSum((1u).xxxx) + (1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupInclusiveAdd_8bbe75());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveAdd_8bbe75() {
  uint4 res = (WavePrefixSum((1u).xxxx) + (1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupInclusiveAdd_8bbe75());
}

