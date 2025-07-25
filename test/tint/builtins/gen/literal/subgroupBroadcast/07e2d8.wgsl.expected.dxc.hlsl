//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupBroadcast_07e2d8() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcast_07e2d8());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupBroadcast_07e2d8() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcast_07e2d8());
}

