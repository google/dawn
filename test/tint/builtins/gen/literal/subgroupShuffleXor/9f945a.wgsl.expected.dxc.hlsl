//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffleXor_9f945a() {
  uint3 res = WaveReadLaneAt((1u).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupShuffleXor_9f945a());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffleXor_9f945a() {
  uint3 res = WaveReadLaneAt((1u).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupShuffleXor_9f945a());
}

