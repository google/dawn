RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_13f36c() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_13f36c();
  return;
}
