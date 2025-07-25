//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveAdd_e18ebb() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveAdd_e18ebb() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_e18ebb()));
}

