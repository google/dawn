
RWByteAddressBuffer prevent_dce : register(u0);
float16_t subgroupShuffleDown_9c6714() {
  float16_t arg_0 = float16_t(1.0h);
  uint arg_1 = 1u;
  float16_t v = arg_0;
  uint v_1 = arg_1;
  float16_t res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffleDown_9c6714());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupShuffleDown_9c6714());
}

