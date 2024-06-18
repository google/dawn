SKIP: FAILED

float4 subgroupBroadcast_b7e93b() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_b7e93b();
}

