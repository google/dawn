//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcast_5196c8() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcast_5196c8() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
}

