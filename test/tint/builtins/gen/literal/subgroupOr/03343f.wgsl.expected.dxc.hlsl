//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupOr_03343f() {
  int3 arg = (int(1)).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(arg)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupOr_03343f() {
  int3 arg = (int(1)).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(arg)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
}

