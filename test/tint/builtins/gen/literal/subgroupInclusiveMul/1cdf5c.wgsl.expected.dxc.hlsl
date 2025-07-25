//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveMul_1cdf5c() {
  uint4 res = (WavePrefixProduct((1u).xxxx) * (1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupInclusiveMul_1cdf5c());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveMul_1cdf5c() {
  uint4 res = (WavePrefixProduct((1u).xxxx) * (1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupInclusiveMul_1cdf5c());
}

