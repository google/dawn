
RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupShuffleDown_b41899() {
  int2 arg_0 = (1).xx;
  uint arg_1 = 1u;
  int2 v = arg_0;
  uint v_1 = arg_1;
  int2 res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_b41899()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffleDown_b41899()));
}

