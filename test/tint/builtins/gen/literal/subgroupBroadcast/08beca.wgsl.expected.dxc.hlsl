RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  prevent_dce.Store(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_08beca();
  return;
}
