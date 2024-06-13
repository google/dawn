float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_08beca()));
  return;
}
