//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupShuffleDown_63fdb0() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleDown_63fdb0());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 3> subgroupShuffleDown_63fdb0() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleDown_63fdb0());
}

