//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupAnd_1877b3() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = asint(WaveActiveBitAnd(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_1877b3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupAnd_1877b3() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = asint(WaveActiveBitAnd(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_1877b3()));
}

