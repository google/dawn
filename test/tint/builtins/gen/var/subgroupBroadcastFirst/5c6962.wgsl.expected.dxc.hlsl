//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupBroadcastFirst_5c6962() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WaveReadLaneFirst(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_5c6962()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupBroadcastFirst_5c6962() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WaveReadLaneFirst(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_5c6962()));
}

