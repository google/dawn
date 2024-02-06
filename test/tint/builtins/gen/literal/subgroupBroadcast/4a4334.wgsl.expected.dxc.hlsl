RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_4a4334() {
  uint2 res = WaveReadLaneAt((1u).xx, 1u);
  prevent_dce.Store2(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_4a4334();
  return;
}
