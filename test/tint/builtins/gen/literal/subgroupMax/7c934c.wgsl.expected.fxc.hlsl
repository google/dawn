SKIP: Wave ops not support before SM6.0

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupMax_7c934c() {
  vector<float16_t, 3> res = WaveActiveMax((float16_t(1.0h)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupMax_7c934c());
  return;
}
