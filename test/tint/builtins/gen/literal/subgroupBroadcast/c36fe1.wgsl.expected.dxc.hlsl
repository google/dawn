RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  prevent_dce.Store(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_c36fe1();
  return;
}
