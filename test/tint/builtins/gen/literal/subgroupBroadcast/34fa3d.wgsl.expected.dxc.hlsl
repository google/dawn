uint3 subgroupBroadcast_34fa3d() {
  uint3 res = WaveReadLaneAt((1u).xxx, 1u);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_34fa3d()));
  return;
}
