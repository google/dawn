RWByteAddressBuffer prevent_dce : register(u0, space2);

void subgroupBroadcast_0f44e2() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> res = WaveReadLaneAt(arg_0, 1u);
  prevent_dce.Store<vector<float16_t, 4> >(0u, res);
}

[numthreads(1, 1, 1)]
void compute_main() {
  subgroupBroadcast_0f44e2();
  return;
}
