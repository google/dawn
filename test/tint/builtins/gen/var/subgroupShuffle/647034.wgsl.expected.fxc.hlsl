SKIP: Wave ops not supported before SM 6.0

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupShuffle_647034() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  int arg_1 = 1;
  vector<float16_t, 4> res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_647034());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupShuffle_647034());
  return;
}
