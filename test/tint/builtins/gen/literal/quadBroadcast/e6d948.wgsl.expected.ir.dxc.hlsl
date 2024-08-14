
RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_e6d948() {
  uint res = QuadReadLaneAt(1u, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

