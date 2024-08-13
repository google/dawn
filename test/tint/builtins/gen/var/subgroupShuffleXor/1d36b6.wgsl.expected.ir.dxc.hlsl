
RWByteAddressBuffer prevent_dce : register(u0);
float subgroupShuffleXor_1d36b6() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float v = arg_0;
  uint v_1 = arg_1;
  float res = WaveReadLaneAt(v, (WaveGetLaneIndex() ^ v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleXor_1d36b6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleXor_1d36b6()));
}

