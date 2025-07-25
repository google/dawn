//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupBroadcastFirst_705aad() {
  vector<float16_t, 4> res = WaveReadLaneFirst((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcastFirst_705aad());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupBroadcastFirst_705aad() {
  vector<float16_t, 4> res = WaveReadLaneFirst((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcastFirst_705aad());
}

