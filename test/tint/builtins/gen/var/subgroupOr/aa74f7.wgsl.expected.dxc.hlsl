//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupOr_aa74f7() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveBitOr(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupOr_aa74f7());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupOr_aa74f7() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveBitOr(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupOr_aa74f7());
}

