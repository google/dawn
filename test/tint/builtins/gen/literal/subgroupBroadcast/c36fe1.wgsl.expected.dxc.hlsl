uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_c36fe1()));
  return;
}
