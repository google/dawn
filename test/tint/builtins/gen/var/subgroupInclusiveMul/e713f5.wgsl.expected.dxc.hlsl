//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupInclusiveMul_e713f5() {
  int2 arg_0 = (1).xx;
  int2 res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupInclusiveMul_e713f5() {
  int2 arg_0 = (1).xx;
  int2 res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
  return;
}
