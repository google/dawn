//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupXor_7f6672() {
  uint2 res = WaveActiveBitXor((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupXor_7f6672());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupXor_7f6672() {
  uint2 res = WaveActiveBitXor((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupXor_7f6672());
}

