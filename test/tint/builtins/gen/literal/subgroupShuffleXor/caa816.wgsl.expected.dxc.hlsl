//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffleXor_caa816() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupShuffleXor_caa816() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
}

