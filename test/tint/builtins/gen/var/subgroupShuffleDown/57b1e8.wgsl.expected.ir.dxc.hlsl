
RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupShuffleDown_57b1e8() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  uint arg_1 = 1u;
  vector<float16_t, 2> v = arg_0;
  uint v_1 = arg_1;
  vector<float16_t, 2> res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffleDown_57b1e8());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupShuffleDown_57b1e8());
}

