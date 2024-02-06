RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_41e5d7() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, 1u);
  prevent_dce.Store<vector<float16_t, 3> >(0u, res);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_41e5d7();
  return;
}
