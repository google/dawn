//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcastFirst_61f177() {
  uint res = WaveReadLaneFirst(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupBroadcastFirst_61f177());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcastFirst_61f177() {
  uint res = WaveReadLaneFirst(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupBroadcastFirst_61f177());
}

