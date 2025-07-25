//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_08beca()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_08beca()));
}

