//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupBroadcast_8855b2() {
  float3 res = WaveReadLaneAt((1.0f).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_8855b2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupBroadcast_8855b2() {
  float3 res = WaveReadLaneAt((1.0f).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_8855b2()));
}

