RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  prevent_dce.Store2(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_3e6879();
  return;
}
