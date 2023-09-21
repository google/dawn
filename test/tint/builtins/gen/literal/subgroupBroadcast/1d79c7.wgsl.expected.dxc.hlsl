RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_1d79c7() {
  int res = WaveReadLaneAt(1, 1u);
  prevent_dce.Store(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_1d79c7();
  return;
}
