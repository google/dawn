SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupMax_33e339() {
  vector<float16_t, 4> res = WaveActiveMax((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupMax_33e339());
  return;
}
