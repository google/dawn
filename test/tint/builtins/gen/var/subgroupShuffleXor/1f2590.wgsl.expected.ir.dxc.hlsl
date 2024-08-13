
RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupShuffleXor_1f2590() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  uint arg_1 = 1u;
  vector<float16_t, 3> v = arg_0;
  uint v_1 = arg_1;
  vector<float16_t, 3> res = WaveReadLaneAt(v, (WaveGetLaneIndex() ^ v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleXor_1f2590());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleXor_1f2590());
}

