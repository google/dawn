//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupXor_9c6e73() {
  int3 arg = (int(1)).xxx;
  int3 res = asint(WaveActiveBitXor(asuint(arg)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_9c6e73()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupXor_9c6e73() {
  int3 arg = (int(1)).xxx;
  int3 res = asint(WaveActiveBitXor(asuint(arg)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_9c6e73()));
}

