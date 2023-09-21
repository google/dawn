RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_1d79c7() {
  int arg_0 = 1;
  int res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store(0u, asuint(res));
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_1d79c7();
  return;
}
