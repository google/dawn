//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupShuffleDown_d269eb() {
  int arg_0 = int(1);
  uint arg_1 = 1u;
  int res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupShuffleDown_d269eb() {
  int arg_0 = int(1);
  uint arg_1 = 1u;
  int res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
}

