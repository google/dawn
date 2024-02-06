RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_279027() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  prevent_dce.Store4(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_279027();
  return;
}
