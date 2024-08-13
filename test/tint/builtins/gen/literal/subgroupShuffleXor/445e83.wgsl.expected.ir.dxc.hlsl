
RWByteAddressBuffer prevent_dce : register(u0);
int subgroupShuffleXor_445e83() {
  int res = WaveReadLaneAt(1, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleXor_445e83()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleXor_445e83()));
}

