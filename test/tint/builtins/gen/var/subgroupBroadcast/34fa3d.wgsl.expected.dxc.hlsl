RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_34fa3d() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store3(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_34fa3d();
  return;
}
