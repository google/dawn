RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_e275c8() {
  int3 res = WaveReadLaneAt((1).xxx, 1u);
  prevent_dce.Store3(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_e275c8();
  return;
}
