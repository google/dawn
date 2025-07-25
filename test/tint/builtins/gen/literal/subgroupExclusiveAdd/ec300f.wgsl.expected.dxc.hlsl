//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveAdd_ec300f() {
  uint4 res = WavePrefixSum((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupExclusiveAdd_ec300f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupExclusiveAdd_ec300f() {
  uint4 res = WavePrefixSum((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupExclusiveAdd_ec300f());
}

