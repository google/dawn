//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffle_85587b() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_85587b()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffle_85587b() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_85587b()));
}

