
RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupInclusiveMul_e713f5() {
  int2 arg_0 = (int(1)).xx;
  int2 v = arg_0;
  int2 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
}

